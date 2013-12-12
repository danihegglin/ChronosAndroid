/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Original file: /Users/danielhegglin/Documents/Android/ChronosAndroid/src/org/ndnx/android/ndnlib/NDNx-Android-Lib/src/org/ndnx/android/ndnlib/IStatusCallback.aidl
 */
package org.ndnx.android.ndnlib;
public interface IStatusCallback extends android.os.IInterface
{
/** Local-side IPC implementation stub class. */
public static abstract class Stub extends android.os.Binder implements org.ndnx.android.ndnlib.IStatusCallback
{
private static final java.lang.String DESCRIPTOR = "org.ndnx.android.ndnlib.IStatusCallback";
/** Construct the stub at attach it to the interface. */
public Stub()
{
this.attachInterface(this, DESCRIPTOR);
}
/**
 * Cast an IBinder object into an org.ndnx.android.ndnlib.IStatusCallback interface,
 * generating a proxy if needed.
 */
public static org.ndnx.android.ndnlib.IStatusCallback asInterface(android.os.IBinder obj)
{
if ((obj==null)) {
return null;
}
android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
if (((iin!=null)&&(iin instanceof org.ndnx.android.ndnlib.IStatusCallback))) {
return ((org.ndnx.android.ndnlib.IStatusCallback)iin);
}
return new org.ndnx.android.ndnlib.IStatusCallback.Stub.Proxy(obj);
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
case TRANSACTION_status:
{
data.enforceInterface(DESCRIPTOR);
int _arg0;
_arg0 = data.readInt();
this.status(_arg0);
reply.writeNoException();
return true;
}
}
return super.onTransact(code, data, reply, flags);
}
private static class Proxy implements org.ndnx.android.ndnlib.IStatusCallback
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
@Override public void status(int st) throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
try {
_data.writeInterfaceToken(DESCRIPTOR);
_data.writeInt(st);
mRemote.transact(Stub.TRANSACTION_status, _data, _reply, 0);
_reply.readException();
}
finally {
_reply.recycle();
_data.recycle();
}
}
}
static final int TRANSACTION_status = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
}
public void status(int st) throws android.os.RemoteException;
}
