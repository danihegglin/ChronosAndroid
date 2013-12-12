/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Original file: /Users/danielhegglin/Documents/Android/ChronosAndroid/src/org/ndnx/android/ndnlib/NDNx-Android-Lib/src/org/ndnx/android/ndnlib/INDNxService.aidl
 */
package org.ndnx.android.ndnlib;
public interface INDNxService extends android.os.IInterface
{
/** Local-side IPC implementation stub class. */
public static abstract class Stub extends android.os.Binder implements org.ndnx.android.ndnlib.INDNxService
{
private static final java.lang.String DESCRIPTOR = "org.ndnx.android.ndnlib.INDNxService";
/** Construct the stub at attach it to the interface. */
public Stub()
{
this.attachInterface(this, DESCRIPTOR);
}
/**
 * Cast an IBinder object into an org.ndnx.android.ndnlib.INDNxService interface,
 * generating a proxy if needed.
 */
public static org.ndnx.android.ndnlib.INDNxService asInterface(android.os.IBinder obj)
{
if ((obj==null)) {
return null;
}
android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
if (((iin!=null)&&(iin instanceof org.ndnx.android.ndnlib.INDNxService))) {
return ((org.ndnx.android.ndnlib.INDNxService)iin);
}
return new org.ndnx.android.ndnlib.INDNxService.Stub.Proxy(obj);
}
@Override public android.os.IBinder asBinder()
{
return this;
}
@Override public boolean onTransact(int code, android.os.Parcel data, android.os.Parcel reply, int flags) throws android.os.RemoteException
{
switch (code)
{
case INTERFACE_TRANSACTION:
{
reply.writeString(DESCRIPTOR);
return true;
}
case TRANSACTION_getStatus:
{
data.enforceInterface(DESCRIPTOR);
int _result = this.getStatus();
reply.writeNoException();
reply.writeInt(_result);
return true;
}
case TRANSACTION_stop:
{
data.enforceInterface(DESCRIPTOR);
this.stop();
reply.writeNoException();
return true;
}
case TRANSACTION_registerStatusCallback:
{
data.enforceInterface(DESCRIPTOR);
org.ndnx.android.ndnlib.IStatusCallback _arg0;
_arg0 = org.ndnx.android.ndnlib.IStatusCallback.Stub.asInterface(data.readStrongBinder());
this.registerStatusCallback(_arg0);
reply.writeNoException();
return true;
}
case TRANSACTION_unregisterStatusCallback:
{
data.enforceInterface(DESCRIPTOR);
org.ndnx.android.ndnlib.IStatusCallback _arg0;
_arg0 = org.ndnx.android.ndnlib.IStatusCallback.Stub.asInterface(data.readStrongBinder());
this.unregisterStatusCallback(_arg0);
reply.writeNoException();
return true;
}
}
return super.onTransact(code, data, reply, flags);
}
private static class Proxy implements org.ndnx.android.ndnlib.INDNxService
{
private android.os.IBinder mRemote;
Proxy(android.os.IBinder remote)
{
mRemote = remote;
}
@Override public android.os.IBinder asBinder()
{
return mRemote;
}
public java.lang.String getInterfaceDescriptor()
{
return DESCRIPTOR;
}
@Override public int getStatus() throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
int _result;
try {
_data.writeInterfaceToken(DESCRIPTOR);
mRemote.transact(Stub.TRANSACTION_getStatus, _data, _reply, 0);
_reply.readException();
_result = _reply.readInt();
}
finally {
_reply.recycle();
_data.recycle();
}
return _result;
}
@Override public void stop() throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
try {
_data.writeInterfaceToken(DESCRIPTOR);
mRemote.transact(Stub.TRANSACTION_stop, _data, _reply, 0);
_reply.readException();
}
finally {
_reply.recycle();
_data.recycle();
}
}
@Override public void registerStatusCallback(org.ndnx.android.ndnlib.IStatusCallback cb) throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
try {
_data.writeInterfaceToken(DESCRIPTOR);
_data.writeStrongBinder((((cb!=null))?(cb.asBinder()):(null)));
mRemote.transact(Stub.TRANSACTION_registerStatusCallback, _data, _reply, 0);
_reply.readException();
}
finally {
_reply.recycle();
_data.recycle();
}
}
@Override public void unregisterStatusCallback(org.ndnx.android.ndnlib.IStatusCallback cb) throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
try {
_data.writeInterfaceToken(DESCRIPTOR);
_data.writeStrongBinder((((cb!=null))?(cb.asBinder()):(null)));
mRemote.transact(Stub.TRANSACTION_unregisterStatusCallback, _data, _reply, 0);
_reply.readException();
}
finally {
_reply.recycle();
_data.recycle();
}
}
}
static final int TRANSACTION_getStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
static final int TRANSACTION_stop = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
static final int TRANSACTION_registerStatusCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
static final int TRANSACTION_unregisterStatusCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
}
public int getStatus() throws android.os.RemoteException;
public void stop() throws android.os.RemoteException;
public void registerStatusCallback(org.ndnx.android.ndnlib.IStatusCallback cb) throws android.os.RemoteException;
public void unregisterStatusCallback(org.ndnx.android.ndnlib.IStatusCallback cb) throws android.os.RemoteException;
}
