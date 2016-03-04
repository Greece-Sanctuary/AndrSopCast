package com.youku.x264;

import java.io.BufferedOutputStream;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.lang.ref.WeakReference;

import android.os.Environment;
import android.os.Message;
import android.util.Log;

public class X264Encoder {
	private native void native_init();
	private native void native_setup(Object week_thiz);
	private native void native_finalize();
	private native void native_setBitrate(int bitrate);
	private native void native_setZerolatencyType(int type);
	private native void native_setResolution(int w, int h, int wd, int hd);
	private native void native_setFps(int fps);
	private native void native_compress(byte[] in, int size, int rotate, int w, int h);
	private native void native_audioInit();
	private native void native_audioSet(int channels, int pCMBitSize, int sampleRate);
	private native void native_audioCompress(byte[] in, int size);
	private static native void native_writeVideoFLV(byte[] in, int size, byte[] tag, int withHeader);
	private static native void native_writeAudioFLV(byte[] in, int size, byte[] tag, int withHeader);
	private static native void native_writeFLV(byte[] in, int size, byte[] tag, int withHeader);
	private native byte[] native_handshake(int streamId, int userID, String token);
	private native int native_checkKey(byte[] in, int size, int streamId);
	private native void native_stop();
	private native void native_prepare();
	
	private File testFile;
	private FileOutputStream outStr;
	private static BufferedOutputStream buff;
	private static boolean ISTEST = true;
	
	public X264Encoder() {
		
		if (ISTEST){
			String sdcardPath = Environment.getExternalStorageDirectory().toString();
			testFile = new File(sdcardPath+"/testxq.flv");
			
			if(testFile.exists()){
				testFile.delete();
			}
			
			try {
				testFile.createNewFile();
				outStr = new FileOutputStream(testFile);
				buff = new BufferedOutputStream(outStr); 
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
		
		native_setup(new WeakReference<X264Encoder>(this));
	}
	
	public void stop() {
		
		if (ISTEST){
			try {
				buff.close();
				outStr.close();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
		native_stop();
	}
	
	public void prepare() {
		native_prepare();
	}
	
	public void init() {
		native_init();
	}
	
	public void setBitrate(int bitrate) {
		native_setBitrate(bitrate);		
	}
	
	public void setZerolatencyType(int zerolatencyType) {
		native_setZerolatencyType(zerolatencyType);
	}

 	public void setResolution(int w, int h, int wd, int hd) {
 		native_setResolution(w, h, wd, hd);
 	}
	
 	public void setFps(int fps) {
 		native_setFps(fps);
 	}
 	
	public void release() {
		native_finalize();
	}

	public void compress(byte[] in, int size, int rotate, int w, int h) {
        native_compress(in, size, rotate, w, h);
    }
	
	public void audioSet(int channels, int pCMBitSize, int sampleRate) {
		native_audioSet(channels, pCMBitSize, sampleRate);
	}
	
	public void audioInit() {
		native_audioInit();
	}
	
	public void audioCompress(byte[] in, int size) {
		native_audioCompress(in, size);
	}
	
	public byte[] handshake(int streamId, int userID, String token) {
		return native_handshake(streamId, userID, token);
	}
	
	public void writeVideoFLV(byte[] in, int size, byte[] tag, int withHeader) {
		native_writeVideoFLV(in, size, tag, withHeader);
	}
	
	public void writeAudioFLV(byte[] in, int size, byte[] tag, int withHeader) {
		native_writeAudioFLV(in, size, tag, withHeader);
	}
	
	private static void postEventFromNative(Object weak_thiz, int msg, byte[] arg0, int arg1, byte[] tag) {
		if(arg0 != null) {
//			Log.v("123", "msg = " + msg + ", arg1 = " + arg1 + ", arg0 length = " + arg0.length);
			if(tag != null) {
//				if(tag[0] == 0)
					native_writeFLV(arg0, arg1, tag, 0);

//				else if(tag[0] == 1 || tag[0] == 2)
//					native_writeFLV(arg0, arg1, tag, 0);
			}
//			Log.v("123", "result len = " + result.length);
		}
		else
			Log.v("123", "msg = " + msg + ", arg1 = " + arg1);
		
		if(msg == 21) {
			Log.i("audio", "msg = " + msg + ", arg1 = " + arg1);
		}
		else if(msg == 11) {
			Log.w("video", "msg = " + msg + ", arg1 = " + arg1);
		}
	}
	
	private static void postVideoPacketEventFromNative(Object weak_thiz, int msg, byte[] arg0) {
		if(arg0 != null) {
//			Log.v("123", "video result len = " + arg0.length);
		}
	}
	
	private static void postAudioPacketEventFromNative(Object weak_thiz, int msg, byte[] arg0) {
		if(arg0 != null) {
//			Log.v("123", "audio result len = " + arg0.length);
		}
	}
	
	private static void postPacketEventFromNative(Object weak_thiz, int msg, byte[] arg0) {
		if(arg0 != null) {
			
			if (ISTEST){
				if (buff != null){
					try {
						buff.write(arg0);
						buff.flush(); 
					} catch (IOException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
			}

//			Log.v("123", "packet result len = " + arg0.length);
		}
	}
	
	static {
		System.loadLibrary("x264Encoder");
	}
}
