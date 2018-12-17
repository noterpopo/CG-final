/*
 * Copyright 2017 Google Inc. All Rights Reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.popo.assimptest.common.helpers;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.provider.Settings;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;

/** Helper to ask camera permission. */
public final class PermissionHelper {
  private static final int CAMERA_PERMISSION_CODE = 0;
  private static final int STOREREAD_PERMISSION_CODE = 1;
  private static final int STOREWRITE_PERMISSION_CODE = 2;
  private static final String[] PERMISSION = {
          Manifest.permission.CAMERA,
          Manifest.permission.READ_EXTERNAL_STORAGE,
          Manifest.permission.WRITE_EXTERNAL_STORAGE};

  /** Check to see we have the necessary permissions for this app. */
  public static boolean hasCameraPermission(Activity activity) {
    return ContextCompat.checkSelfPermission(activity, PERMISSION[0])
        == PackageManager.PERMISSION_GRANTED;
  }
  public static boolean hasStoreReadPermission(Activity activity) {
    return ContextCompat.checkSelfPermission(activity, PERMISSION[1])
            == PackageManager.PERMISSION_GRANTED;
  }
  public static boolean hasStoreWritePermission(Activity activity) {
    return ContextCompat.checkSelfPermission(activity, PERMISSION[2])
            == PackageManager.PERMISSION_GRANTED;
  }

  /** Check to see we have the necessary permissions for this app, and ask for them if we don't. */
  public static void requestCameraPermission(Activity activity) {
    ActivityCompat.requestPermissions(
        activity, new String[] {PERMISSION[0]}, CAMERA_PERMISSION_CODE);
  }
  public static void requestStoreReadPermission(Activity activity) {
    ActivityCompat.requestPermissions(
            activity, new String[] {PERMISSION[1]}, STOREREAD_PERMISSION_CODE);
  }
  public static void requestStoreWritePermission(Activity activity) {
    ActivityCompat.requestPermissions(
            activity, new String[] {PERMISSION[2]}, STOREWRITE_PERMISSION_CODE);
  }

  /** Check to see if we need to show the rationale for this permission. */
  public static boolean shouldShowCameraRequestPermissionRationale(Activity activity) {
    return ActivityCompat.shouldShowRequestPermissionRationale(activity, PERMISSION[0]);
  }
  public static boolean shouldShowSDCardRRequestPermissionRationale(Activity activity) {
    return ActivityCompat.shouldShowRequestPermissionRationale(activity, PERMISSION[1]);
  }
  public static boolean shouldShowSDCardWRequestPermissionRationale(Activity activity) {
    return ActivityCompat.shouldShowRequestPermissionRationale(activity, PERMISSION[2]);
  }

  /** Launch Application Setting to grant permission. */
  public static void launchPermissionSettings(Activity activity) {
    Intent intent = new Intent();
    intent.setAction(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
    intent.setData(Uri.fromParts("package", activity.getPackageName(), null));
    activity.startActivity(intent);
  }
}
