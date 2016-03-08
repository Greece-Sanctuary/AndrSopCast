package com.stainberg.x264;

import java.io.IOException;
import java.util.concurrent.ArrayBlockingQueue;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.content.res.Configuration;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.hardware.Camera.Parameters;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.media.MediaRecorder;
import android.os.Build;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;
import android.view.SurfaceView;
import android.view.Window;
import android.view.WindowManager;

import com.youku.x264.X264Encoder;

/**
 * Created by stainberg on 4/7/15.
 */
public class AvCapture extends Activity {

	 private X264Encoder x264Encoder = null;
	    private static final int WWW = 720;
	    private static final int HHH = 1280;
	    private SurfaceHolder mSurfaceHolder = null;
	    private Camera mCamera = null;
	    private int mCameraId = 0;
	    private static final String TAG = AvCapture.class.getSimpleName();
	    private boolean bIfPreview = false;
	    private static final int pixWidth = WWW;
	    private static final int pixHeight = HHH;

	    private static final int BITRATE = 500;
	    private static final int FPS = 15;
	    private static final int frequency = 44100;
	    private static final int channelConfiguration = AudioFormat.CHANNEL_OUT_STEREO;
	    private static final int audioEncoding = AudioFormat.ENCODING_PCM_16BIT;
	    private boolean startAudio;
	    private boolean audioIntrp;
	    private boolean videoIntrp;
	    private AudioRecord audioRecord;
	    private static int recBufSize;
	    private byte[] recBuf;
	    public static ArrayBlockingQueue<avcDataInfo> YUVQueue = new ArrayBlockingQueue<avcDataInfo>(100); 
		
		private AvcEncoder avcCodec;
	    
		public static class avcDataInfo{
			public byte[] mData;
			public byte[] mTag;
			public avcDataInfo(byte[] data, byte[] tag){
				mData = data;
				mTag = tag;
			}
		}
		
	    private Thread audioThread = new Thread() {
	        public void run() {
	            while(!audioIntrp) {
	                while(!startAudio) {
	                    try {
	                        Thread.sleep(1000);
	                    } catch (InterruptedException e) {
	                        e.printStackTrace();
	                    }
	                }
	            //start recorder
	                if(x264Encoder != null) {
		                int readLen = audioRecord.read(recBuf, 0, recBufSize);
		                x264Encoder.audioCompress(recBuf, readLen);
	                }
//	                Log.w(TAG, "audio readLen: " + readLen);
	            }
	            //TODO
	            //stop recorder
	            if(null != audioRecord) {
	                audioRecord.stop();
	                audioRecord.release();
	                audioRecord = null;
	            }
	        }
	    };

	    public AvCapture() {
	    	
	    }
	    
	    public void onCreate(Bundle savedInstanceState) {
	        super.onCreate(savedInstanceState);
	        
	        this.requestWindowFeature(Window.FEATURE_NO_TITLE);//去掉标题栏

	        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);//去掉信息栏
	        
	        setContentView(R.layout.activity_av_capture);
	        audioIntrp = false;
	        startAudio = false;
	        videoIntrp = true;
	        setCameraId();
	        audioThread.start();

	    }

	    public static  int getScreenWidth(Context context) {
	        DisplayMetrics metric = new DisplayMetrics();
	        WindowManager wm = (WindowManager) context
	                .getSystemService(Context.WINDOW_SERVICE);
	        wm.getDefaultDisplay().getMetrics(metric);
	        int width = metric.widthPixels;
	        return width;
	    }

	    private void initSurfaceView() {
	        SurfaceView mSurfaceView = (SurfaceView)findViewById(R.id.tablet_camera_view);
	        mSurfaceHolder = mSurfaceView.getHolder();
	        mSurfaceHolder.addCallback(mSurfaceHolderCallback);
	        mSurfaceHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
	        
	    }

	    public void onStart() {
	        super.onStart();
	        initSurfaceView();
	    }

	    public void onStop() {
	        super.onStop();
	        if(x264Encoder != null) {
                x264Encoder.release();
            }
	    }

	    private X264Encoder.LFLiveCallback m_callback = new X264Encoder.LFLiveCallback() {
		

			@Override
			public void CallBackAVCFromNative(byte[] arg0, int arg1, int width,
					int height, byte[] tag) {
				putYUVData(arg0,arg1,tag);
			}
		};
	    
	    private Callback mSurfaceHolderCallback = new Callback() {
	        @Override
	        public void surfaceDestroyed(SurfaceHolder holder) {
	        	if(null != mCamera) {
	                mCamera.setPreviewCallback(null); //！！这个必须在前，不然退出出错
	                mCamera.stopPreview();
	                bIfPreview = false;
	                mCamera.release();
	                mCamera = null;
	            }
	        	avcCodec.StopThread();
	        	videoIntrp = true;
	        	audioIntrp = true;
	            startAudio = false;
	            x264Encoder.stop();
	        }

	        @TargetApi(Build.VERSION_CODES.GINGERBREAD)
	        @Override
	        public void surfaceCreated(SurfaceHolder holder) {
	            x264Encoder = new X264Encoder(SupportAvcCodec());
	            x264Encoder.setFps(FPS);
	            x264Encoder.setBitrate(BITRATE);
	            x264Encoder.setResolution(1280/2, 720/2, 9, 16);
	            x264Encoder.init();
	            x264Encoder.setCallback(m_callback);
		        
		        avcCodec = new AvcEncoder(720/2,1280/2,15,300*1000);
		        avcCodec.setx264Encoder(x264Encoder);
				avcCodec.StartEncoderThread();
	            
	            mCamera = Camera.open(mCameraId);
	            try {
	                mCamera.setPreviewDisplay(mSurfaceHolder);
	            } catch (IOException e) {
	                e.printStackTrace();
	            }
	            recBufSize = AudioRecord.getMinBufferSize(frequency, channelConfiguration, audioEncoding);
	            recBuf = new byte[recBufSize];
	            audioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC, frequency, channelConfiguration, audioEncoding, recBufSize);
	            audioRecord.startRecording();
	            x264Encoder.audioSet(audioRecord.getChannelCount(), 16, frequency);//audioRecord.getChannelCount()
	            x264Encoder.audioInit();
	            startAudio = true;
	            videoIntrp = false;
	            x264Encoder.prepare();
	        }
	        @Override
	        public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
	            initCamera();
	        }
	    };

	    @TargetApi(Build.VERSION_CODES.GINGERBREAD)
	    public void setCameraId() {
	        Camera.CameraInfo cameraInfo = new Camera.CameraInfo();
	        int numberOfCameras = Camera.getNumberOfCameras();
	        for (int i=0;i<numberOfCameras;i++) {
	            Camera.getCameraInfo(i, cameraInfo);
	            if (cameraInfo.facing == Camera.CameraInfo.CAMERA_FACING_BACK) {
	                mCameraId = i;
	                break;
	            }
	        }
	    }

	    private void initCamera() {
	        Log.i(TAG, "going into initCamera");
	        if (bIfPreview) {
	            mCamera.stopPreview();
	        }
	        if(null != mCamera) {
	            try {
	                Camera.Parameters parameters = mCamera.getParameters();
	                parameters.setFlashMode("off");
	                parameters.setFocusMode(Parameters.FOCUS_MODE_CONTINUOUS_VIDEO);
	                parameters.setPictureFormat(ImageFormat.JPEG);
	                parameters.setPreviewFormat(ImageFormat.NV21);
	                parameters.setPreviewFrameRate(FPS);
	                parameters.setPictureSize(HHH, WWW);
	                parameters.setPreviewSize(HHH, WWW);
	                if (this.getResources().getConfiguration().orientation != Configuration.ORIENTATION_LANDSCAPE) {
	                    parameters.set("orientation", "portrait");
	                    parameters.set("rotation", 90);
	                    mCamera.setDisplayOrientation(90);
	                } else {
	                    parameters.set("orientation", "landscape");
	                    mCamera.setDisplayOrientation(0);
	                }
	                mCamera.setPreviewCallback(mPreviewCallback);
	                mCamera.setParameters(parameters);
	                mCamera.startPreview();
	                bIfPreview = true;
	                Camera.Size csize = mCamera.getParameters().getPreviewSize();
//	                Log.i(TAG+"initCamera", "after setting, previewSize:width: " + csize.width + " height: " + csize.height);
	                csize = mCamera.getParameters().getPictureSize();
//	                Log.i(TAG+"initCamera", "after setting, pictruesize:width: " + csize.width + " height: " + csize.height);
//	                Log.i(TAG+"initCamera", "after setting, previewformate is " + mCamera.getParameters().getPreviewFormat());
//	                Log.i(TAG + "initCamera", "after setting, previewframetate is " + mCamera.getParameters().getPreviewFrameRate());
	            } catch (Exception e) {
	                e.printStackTrace();
	            }
	        }
	    }

	    private Camera.PreviewCallback mPreviewCallback = new Camera.PreviewCallback() {
	        @Override
	        public void onPreviewFrame(byte[] data, Camera camera) {
	        	if(videoIntrp) {
	        		return;
	        	}
	        	if(data.length > 0) {
	        		x264Encoder.compress(data, data.length, 90, HHH, WWW);
	        	}
	            
	        }
	    };
	    
	    @SuppressLint("NewApi")
		private boolean SupportAvcCodec(){
			if(Build.VERSION.SDK_INT>=18){
				for(int j = MediaCodecList.getCodecCount() - 1; j >= 0; j--){
					MediaCodecInfo codecInfo = MediaCodecList.getCodecInfoAt(j);
		
					String[] types = codecInfo.getSupportedTypes();
					for (int i = 0; i < types.length; i++) {
						if (types[i].equalsIgnoreCase("video/avc")) {
							return true;
						}
					}
				}
			}
			return false;
		}
	    
	    public void putYUVData(byte[] buffer, int length, byte[] tag) {
			if (YUVQueue.size() >= 10) {
				YUVQueue.poll();
			}
			YUVQueue.add(new avcDataInfo(buffer, tag));
		}
}
