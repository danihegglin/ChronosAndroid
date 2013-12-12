/* -*- Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2012 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Zhenkai Zhu <zhenkai@cs.ucla.edu>
 *         Chaoyi Bian <bcy@pku.edu.cn>
 *	   Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#include "sync-ccnx-wrapper.h"

extern "C" {
#include <ccn/fetch.h>
}

#include "sync-logging.h"
#include <poll.h>
#include <boost/throw_exception.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/random.hpp>

#include "sync-scheduler.h"

typedef boost::error_info<struct tag_errmsg, std::string> errmsg_info_str;
typedef boost::error_info<struct tag_errmsg, int> errmsg_info_int;


using namespace std;
using namespace boost;

INIT_LOGGER ("CcnxWrapper");

namespace Sync {

#ifdef _DEBUG_WRAPPER_      
CcnxWrapper::CcnxWrapper(char c)
#else
CcnxWrapper::CcnxWrapper()
#endif
  : m_handle (0)
  , m_keyStore (0)
  , m_keyLoactor (0)
  , m_running (true)
  , m_connected (false)
{
#ifdef _DEBUG_WRAPPER_      
  m_c = c;
#endif
  connectCcnd();
  initKeyStore ();
  createKeyLocator ();
  m_thread = thread (&CcnxWrapper::ccnLoop, this);
}

void
CcnxWrapper::connectCcnd()
{
  recursive_mutex::scoped_lock lock (m_mutex);

  if (m_handle != 0) {
    ccn_disconnect (m_handle);
    ccn_destroy (&m_handle);
  }
  
  m_handle = ccn_create ();
  _LOG_DEBUG("<<< connecting to ccnd");
  if (ccn_connect(m_handle, NULL) < 0)
  {
    _LOG_DEBUG("<<< connecting to ccnd failed");
    BOOST_THROW_EXCEPTION (CcnxOperationException() << errmsg_info_str("connection to ccnd failed"));
  }
  m_connected = true;

  if (!m_registeredInterests.empty())
  {
    for (map<std::string, InterestCallback>::const_iterator it = m_registeredInterests.begin(); it != m_registeredInterests.end(); ++it)
    {
      // clearInterestFilter(it->first);
      setInterestFilter(it->first, it->second);
      _LOG_DEBUG("<<< registering interest filter for: " << it->first);
    }
  }
}

CcnxWrapper::~CcnxWrapper()
{
  // std::cout << "CcnxWrapper::~CcnxWrapper()" << std::endl;
  {
    recursive_mutex::scoped_lock lock(m_mutex);
    m_running = false;
  }
  
  m_thread.join ();
  ccn_disconnect (m_handle);
  ccn_destroy (&m_handle);
  ccn_charbuf_destroy (&m_keyLoactor);
  ccn_keystore_destroy (&m_keyStore);
}

/// @cond include_hidden

void
CcnxWrapper::createKeyLocator ()
{
  m_keyLoactor = ccn_charbuf_create();
  ccn_charbuf_append_tt (m_keyLoactor, CCN_DTAG_KeyLocator, CCN_DTAG);
  ccn_charbuf_append_tt (m_keyLoactor, CCN_DTAG_Key, CCN_DTAG);
  int res = ccn_append_pubkey_blob (m_keyLoactor, ccn_keystore_public_key(m_keyStore));
  if (res >= 0)
    {
      ccn_charbuf_append_closer (m_keyLoactor); /* </Key> */
      ccn_charbuf_append_closer (m_keyLoactor); /* </KeyLocator> */
    }
}

const ccn_pkey*
CcnxWrapper::getPrivateKey ()
{
  return ccn_keystore_private_key (m_keyStore);
}

const unsigned char*
CcnxWrapper::getPublicKeyDigest ()
{
  return ccn_keystore_public_key_digest(m_keyStore);
}

ssize_t
CcnxWrapper::getPublicKeyDigestLength ()
{
  return ccn_keystore_public_key_digest_length(m_keyStore);
}

void
CcnxWrapper::initKeyStore ()
{
  m_keyStore = ccn_keystore_create ();
  string keyStoreFile = string(getenv("HOME")) + string("/.ccnx/.ccnx_keystore");
  if (ccn_keystore_init (m_keyStore, (char *)keyStoreFile.c_str(), (char*)"Th1s1sn0t8g00dp8ssw0rd.") < 0)
    BOOST_THROW_EXCEPTION(CcnxOperationException() << errmsg_info_str(keyStoreFile.c_str()));
}

void
CcnxWrapper::ccnLoop ()
{
  _LOG_FUNCTION (this);
  static boost::mt19937 randomGenerator (static_cast<unsigned int> (std::time (0)));
  static boost::variate_generator<boost::mt19937&, boost::uniform_int<> > rangeUniformRandom (randomGenerator, uniform_int<> (0,1000));

  while (m_running)
    {
      try
        {
#ifdef _DEBUG_WRAPPER_      
          std::cout << m_c << flush;
#endif
          int res = 0;
          {
            recursive_mutex::scoped_lock lock (m_mutex);
            res = ccn_run (m_handle, 0);
          }

          if (!m_running) break;
        
          if (res < 0)
            BOOST_THROW_EXCEPTION (CcnxOperationException()
                                 << errmsg_info_str("ccn_run returned error"));


          pollfd pfds[1];
          {
            recursive_mutex::scoped_lock lock (m_mutex);
          
            pfds[0].fd = ccn_get_connection_fd (m_handle);
            pfds[0].events = POLLIN;
            if (ccn_output_is_pending (m_handle))
              pfds[0].events |= POLLOUT;
          }
        
          int ret = poll (pfds, 1, 1);
          if (ret < 0)
            {
              BOOST_THROW_EXCEPTION (CcnxOperationException() << errmsg_info_str("ccnd socket failed (probably ccnd got stopped)"));
            }
        }
        catch (CcnxOperationException &e)
        {
          // do not try reconnect for now
          throw e;
          /*
          m_connected = false;
          // probably ccnd has been stopped
          // try reconnect with sleep
          int interval = 1;
          int maxInterval = 32;
          while (m_running)
          {
            try
            {
              this_thread::sleep (boost::get_system_time () + TIME_SECONDS(interval) + TIME_MILLISECONDS (rangeUniformRandom ()));

              connectCcnd();
              _LOG_DEBUG("reconnect to ccnd succeeded");
              break;
            }
            catch (CcnxOperationException &e)
            {
              this_thread::sleep (boost::get_system_time () + TIME_SECONDS(interval) + TIME_MILLISECONDS (rangeUniformRandom ()));

              // do exponential backup for reconnect interval
              if (interval < maxInterval)
              {
                interval *= 2;
              }
            }
          }
          */
        }
        catch (const std::exception &exc)
          {
            // catch anything thrown within try block that derives from std::exception
            std::cerr << exc.what();
          }
        catch (...)
          {
            cout << "UNKNOWN EXCEPTION !!!" << endl; 
          }
          
     } 
}

/// @endcond
int
CcnxWrapper::publishStringData (const string &name, const string &dataBuffer, int freshness) {
  return publishRawData(name, dataBuffer.c_str(), dataBuffer.length(), freshness);
}

int
CcnxWrapper::publishRawData (const string &name, const char *buf, size_t len, int freshness)
{

  recursive_mutex::scoped_lock lock(m_mutex);
  if (!m_running || !m_connected)
    return -1;
  
  // cout << "Publish: " << name << endl;
  ccn_charbuf *pname = ccn_charbuf_create();
  ccn_charbuf *signed_info = ccn_charbuf_create();
  ccn_charbuf *content = ccn_charbuf_create();

  ccn_name_from_uri(pname, name.c_str());
  ccn_signed_info_create(signed_info,
			 getPublicKeyDigest(),
			 getPublicKeyDigestLength(),
			 NULL,
			 CCN_CONTENT_DATA,
			 freshness,
			 NULL,
			 m_keyLoactor);
  if(ccn_encode_ContentObject(content, pname, signed_info,
			   (const unsigned char *)buf, len,
			   NULL, getPrivateKey()) < 0)
  {
    // BOOST_THROW_EXCEPTION(CcnxOperationException() << errmsg_info_str("encode content failed"));
    _LOG_ERROR("<<< Encode content failed " << name);
  }

  if (ccn_put(m_handle, content->buf, content->length) < 0)
  {
    // BOOST_THROW_EXCEPTION(CcnxOperationException() << errmsg_info_str("ccnput failed"));
    _LOG_ERROR("<<< ccnput content failed " << name);
  }

  ccn_charbuf_destroy (&pname);
  ccn_charbuf_destroy (&signed_info);
  ccn_charbuf_destroy (&content);
  return 0;
}


static ccn_upcall_res
incomingInterest(ccn_closure *selfp,
                 ccn_upcall_kind kind,
                 ccn_upcall_info *info)
{
  CcnxWrapper::InterestCallback *f = static_cast<CcnxWrapper::InterestCallback*> (selfp->data);

  switch (kind)
    {
    case CCN_UPCALL_FINAL: // effective in unit tests
      delete f;
      delete selfp;
      return CCN_UPCALL_RESULT_OK;

    case CCN_UPCALL_INTEREST:
      break;

    default:
      return CCN_UPCALL_RESULT_OK;
    }

  string interest;
  for (int i = 0; i < info->interest_comps->n - 1; i++)
    {
      char *comp;
      size_t size;
      interest += "/";
      ccn_name_comp_get(info->interest_ccnb, info->interest_comps, i, (const unsigned char **)&comp, &size);
      string compStr(comp, size);
      interest += compStr;
    }
  (*f) (interest);
  _LOG_DEBUG("<<< processed interest: " << interest);
  return CCN_UPCALL_RESULT_OK;
}

static ccn_upcall_res
incomingData(ccn_closure *selfp,
             ccn_upcall_kind kind,
             ccn_upcall_info *info)
{
  ClosurePass *cp = static_cast<ClosurePass *> (selfp->data);

  switch (kind)
    {
    case CCN_UPCALL_FINAL:  // effecitve in unit tests
      delete cp;
      cp = NULL;
      delete selfp;
      return CCN_UPCALL_RESULT_OK;

    case CCN_UPCALL_CONTENT:
      break;

    case CCN_UPCALL_INTEREST_TIMED_OUT: {
      if (cp != NULL && cp->getRetry() > 0) {
        cp->decRetry();
        return CCN_UPCALL_RESULT_REEXPRESS;
      }
      return CCN_UPCALL_RESULT_OK;
    }

    default:
      return CCN_UPCALL_RESULT_OK;
    }

  char *pcontent;
  size_t len;
  if (ccn_content_get_value(info->content_ccnb, info->pco->offset[CCN_PCO_E], info->pco, (const unsigned char **)&pcontent, &len) < 0)
  {
    // BOOST_THROW_EXCEPTION(CcnxOperationException() << errmsg_info_str("decode ContentObject failed"));
    _LOG_ERROR("<<< Decode content failed ");
  }

  string name;
  for (int i = 0; i < info->content_comps->n - 1; i++)
    {
      char *comp;
      size_t size;
      name += "/";
      ccn_name_comp_get(info->content_ccnb, info->content_comps, i, (const unsigned char **)&comp, &size);
      string compStr(comp, size);
      name += compStr;
    }

  cp->runCallback(name, pcontent, len);

  return CCN_UPCALL_RESULT_OK;
}

int CcnxWrapper::sendInterestForString (const string &strInterest, const StringDataCallback &strDataCallback, int retry)
{
  DataClosurePass * pass = new DataClosurePass(STRING_FORM, retry, strDataCallback);
  return sendInterest(strInterest, pass);
}

int CcnxWrapper::sendInterest (const string &strInterest, const RawDataCallback &rawDataCallback, int retry)
{
  RawDataClosurePass * pass = new RawDataClosurePass(RAW_DATA, retry, rawDataCallback);
  return sendInterest(strInterest, pass);
}

int CcnxWrapper::sendInterest (const string &strInterest, void *dataPass)
{
  recursive_mutex::scoped_lock lock(m_mutex);
  if (!m_running || !m_connected)
    return -1;
  
  // std::cout << "Send interests for " << strInterest << std::endl;
  ccn_charbuf *pname = ccn_charbuf_create();
  ccn_closure *dataClosure = new ccn_closure;

  ccn_name_from_uri (pname, strInterest.c_str());
  dataClosure->data = dataPass;

  dataClosure->p = &incomingData;
  if (ccn_express_interest (m_handle, pname, dataClosure, NULL) < 0)
  {
    // BOOST_THROW_EXCEPTION(CcnxOperationException() << errmsg_info_str("express interest failed"));
    _LOG_ERROR("<<< Express interest failed: " << strInterest);
  }

  _LOG_DEBUG("<<< Sending interest: " << strInterest);

  ccn_charbuf_destroy (&pname);
  return 0;
}

int CcnxWrapper::setInterestFilter (const string &prefix, const InterestCallback &interestCallback)
{
  recursive_mutex::scoped_lock lock(m_mutex);
  if (!m_running || !m_connected)
    return -1;

  ccn_charbuf *pname = ccn_charbuf_create();
  ccn_closure *interestClosure = new ccn_closure;

  ccn_name_from_uri (pname, prefix.c_str());
  interestClosure->data = new InterestCallback (interestCallback); // should be removed when closure is removed
  interestClosure->p = &incomingInterest;
  int ret = ccn_set_interest_filter (m_handle, pname, interestClosure);
  if (ret < 0)
    {
      BOOST_THROW_EXCEPTION(CcnxOperationException() << errmsg_info_str("set interest filter failed") << errmsg_info_int (ret));
    }

  m_registeredInterests.insert(pair<std::string, InterestCallback>(prefix, interestCallback));
  ccn_charbuf_destroy(&pname);

  return 0;
}

void
CcnxWrapper::clearInterestFilter (const std::string &prefix)
{
  recursive_mutex::scoped_lock lock(m_mutex);
  if (!m_running || !m_connected)
    return;

  std::cout << "clearInterestFilter" << std::endl;
  ccn_charbuf *pname = ccn_charbuf_create();

  ccn_name_from_uri (pname, prefix.c_str());
  int ret = ccn_set_interest_filter (m_handle, pname, 0);
  if (ret < 0)
    {
      BOOST_THROW_EXCEPTION(CcnxOperationException() << errmsg_info_str("set interest filter failed") << errmsg_info_int (ret));
    }

  m_registeredInterests.erase(prefix);
  ccn_charbuf_destroy(&pname);
}

string
CcnxWrapper::getLocalPrefix ()
{
  struct ccn * tmp_handle = ccn_create ();
  int res = ccn_connect (tmp_handle, NULL);
  if (res < 0)
    {
      _LOG_ERROR ("connecting to ccnd failed");
      return "";
    }
  
  string retval = "";
  
  struct ccn_charbuf *templ = ccn_charbuf_create();
  ccn_charbuf_append_tt(templ, CCN_DTAG_Interest, CCN_DTAG);
  ccn_charbuf_append_tt(templ, CCN_DTAG_Name, CCN_DTAG);
  ccn_charbuf_append_closer(templ); /* </Name> */
  // XXX - use pubid if possible
  ccn_charbuf_append_tt(templ, CCN_DTAG_MaxSuffixComponents, CCN_DTAG);
  ccnb_append_number(templ, 1);
  ccn_charbuf_append_closer(templ); /* </MaxSuffixComponents> */
  ccnb_tagged_putf(templ, CCN_DTAG_Scope, "%d", 2);
  ccn_charbuf_append_closer(templ); /* </Interest> */

  struct ccn_charbuf *name = ccn_charbuf_create ();
  res = ccn_name_from_uri (name, "/local/ndn/prefix");
  if (res < 0) {
    _LOG_ERROR ("Something wrong with `ccn_name_from_uri` call");
  }
  else
    {
      struct ccn_fetch *fetch = ccn_fetch_new (tmp_handle);
      
      struct ccn_fetch_stream *stream = ccn_fetch_open (fetch, name, "/local/ndn/prefix",
                                                        NULL, 4, CCN_V_HIGHEST, 0);      
      if (stream == NULL) {
        _LOG_ERROR ("Cannot create ccn_fetch_stream");
      }
      else
        {
          ostringstream os;

          int counter = 0;
          char buf[256];
          while (true) {
            res = ccn_fetch_read (stream, buf, sizeof(buf));
            
            if (res == 0) {
              break;
            }
            
            if (res > 0) {
              os << string(buf, res);
            } else if (res == CCN_FETCH_READ_NONE) {
              if (counter < 2)
                {
                  ccn_run(tmp_handle, 1000);
                  counter ++;
                }
              else
                {
                  break;
                }
            } else if (res == CCN_FETCH_READ_END) {
              break;
            } else if (res == CCN_FETCH_READ_TIMEOUT) {
              break;
            } else {
              _LOG_ERROR ("fatal error. should be reported");
              break;
            }
          }
          retval = os.str ();
          stream = ccn_fetch_close(stream);
        }
      fetch = ccn_fetch_destroy(fetch);
    }

  ccn_charbuf_destroy (&name);

  ccn_disconnect (tmp_handle);
  ccn_destroy (&tmp_handle);
  
  return retval;
}





DataClosurePass::DataClosurePass (CallbackType type, int retry, const CcnxWrapper::StringDataCallback &strDataCallback): ClosurePass(type, retry), m_callback(NULL)
{
   m_callback = new CcnxWrapper::StringDataCallback (strDataCallback); 
}

DataClosurePass::~DataClosurePass () 
{
  delete m_callback;
  m_callback = NULL;
}

void 
DataClosurePass::runCallback(std::string name, const char *data, size_t len) 
{
  string content(data, len);
  if (m_callback != NULL) {
    (*m_callback)(name, content);
  }
}


RawDataClosurePass::RawDataClosurePass (CallbackType type, int retry, const CcnxWrapper::RawDataCallback &rawDataCallback): ClosurePass(type, retry), m_callback(NULL)
{
   m_callback = new CcnxWrapper::RawDataCallback (rawDataCallback); 
}

RawDataClosurePass::~RawDataClosurePass () 
{
  delete m_callback;
  m_callback = NULL;
}

void 
RawDataClosurePass::runCallback(std::string name, const char *data, size_t len) 
{
  if (m_callback != NULL) {
    (*m_callback)(name, data, len);
  }
}

}
