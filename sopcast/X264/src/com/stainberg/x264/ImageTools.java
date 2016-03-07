package com.stainberg.x264;

import android.graphics.Bitmap;
import android.graphics.ImageFormat;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;

/**
 * @author AleXQ
 * @Date 16/3/5
 * @Description:
 */

public class ImageTools {

	private static final String TAG = "ImageTools";

	/*
		将YUV数据保存成JPEG图片
		@Param filePath:        文件保存路径
		@Param fileNameNoExp:   文件名称不包含扩展名
		@Param yuvData:         YUV数据
		@Param width:           图片宽
		@Param height:          图片高
		@return 存储是否成功
	 */
	public static boolean saveYUVtoJPEGFile(String filePath, String fileNameNoExp, byte[] yuvData, int width, int height) {
		Log.d(TAG, "saveYUVtoJPEGFile  "
						+ "filePath:" + filePath
						+ ",fileNameNoExp:" + fileNameNoExp
						+ ",width:" + width
						+ ",height:" + height
		);
		boolean bret = false;
		try {
			File mkDir = new File(filePath);
			if (!mkDir.exists()) {
				mkDir.mkdirs();
			}

			File picFile = new File(filePath, fileNameNoExp + ".jpeg");

			if (picFile.exists()) {
				picFile.delete();
			}
			picFile.createNewFile();
			FileOutputStream filecon = new FileOutputStream(picFile);
			YuvImage image = new YuvImage(yuvData, ImageFormat.NV21, width, height, null);
			bret = image.compressToJpeg(new Rect(0, 0, width, height), 100, filecon);
			filecon.flush();
			filecon.close();

		} catch (Exception e) {
			e.printStackTrace();
		}
		return bret;
	}

	/*
		将RGB数据保存成PNG图片
		@Param filePath:        文件保存路径
		@Param fileNameNoExp:   文件名称不包含扩展名
		@Param rgbColos:        RGB数据
		@Param width:           图片宽
		@Param height:          图片高
		@return 存储是否成功
	 */
	public static boolean saveRGBtoPNGFile(String filePath, String fileNameNoExp, byte[] rgbColos, int width, int height) {
		Log.d(TAG, "saveRGBtoPNGFile  "
						+ "filePath:" + filePath
						+ ",fileNameNoExp:" + fileNameNoExp
						+ ",width:" + width
						+ ",height:" + height
		);
		boolean bret = false;
		try {
			File mkDir = new File(filePath);
			if (!mkDir.exists()) {
				mkDir.mkdirs();
			}

			File picFile = new File(filePath, fileNameNoExp + ".png");

			if (picFile.exists()) {
				picFile.delete();
			}
			FileOutputStream out = new FileOutputStream(picFile);

			Bitmap bitmap = Bitmap.createBitmap(convertByteToColor(rgbColos), width, height, Bitmap.Config.RGB_565);

			bret = bitmap.compress(Bitmap.CompressFormat.PNG, 100, out);

			out.flush();
			out.close();

		} catch (Exception e) {
			e.printStackTrace();
		}
		return bret;
	}


	/*
	 * 将RGB数组转化为像素数组
	 */
	private static int[] convertByteToColor(byte[] data){
		int size = data.length;
		if (size == 0){
			return null;
		}


		// 理论上data的长度应该是3的倍数，这里做个兼容
		int arg = 0;
		if (size % 3 != 0){
			arg = 1;
		}

		int []color = new int[size / 3 + arg];
		int red, green, blue;


		if (arg == 0){											//  正好是3的倍数
			for(int i = 0; i < color.length; ++i){

				color[i] = (data[i * 3] << 16 & 0x00FF0000) |
						(data[i * 3 + 1] << 8 & 0x0000FF00 ) |
						(data[i * 3 + 2] & 0x000000FF ) |
						0xFF000000;
			}
		}else{													// 不是3的倍数
			for(int i = 0; i < color.length - 1; ++i){
				color[i] = (data[i * 3] << 16 & 0x00FF0000) |
						(data[i * 3 + 1] << 8 & 0x0000FF00 ) |
						(data[i * 3 + 2] & 0x000000FF ) |
						0xFF000000;
			}

			color[color.length - 1] = 0xFF000000;					// 最后一个像素用黑色填充
		}

		return color;
	}

}
