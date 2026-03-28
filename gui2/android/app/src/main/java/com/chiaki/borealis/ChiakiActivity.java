// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL

package com.chiaki.borealis;

import android.content.res.AssetManager;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;

import org.libsdl.app.SDL;
import org.libsdl.app.SDLActivity;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class ChiakiActivity extends SDLActivity {

    private static final String TAG = "chiaki-borealis";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // Extract assets before SDL starts (SDL_main needs resources)
        String resPath = extractAssets();
        String configDir = getFilesDir().getAbsolutePath() + "/";

        // Load native libraries manually so nativeSetenv is available
        // before super.onCreate starts the SDL thread
        for (String lib : getLibraries()) {
            SDL.loadLibrary(lib);
        }

        // Set environment variables BEFORE super.onCreate starts the SDL thread
        nativeSetenv("BOREALIS_RESOURCES", resPath);
        nativeSetenv("CHIAKI_CONFIG_DIR", configDir);

        Log.i(TAG, "ChiakiActivity created, resources at: " + resPath);

        super.onCreate(savedInstanceState);
    }

    private String extractAssets() {
        String destDir = getFilesDir().getAbsolutePath() + "/res/";
        File dest = new File(destDir);

        // Check if already extracted (simple marker file)
        File marker = new File(destDir, ".extracted");
        if (marker.exists()) {
            Log.i(TAG, "Assets already extracted");
            return destDir;
        }

        Log.i(TAG, "Extracting assets to " + destDir);
        try {
            copyAssetDir(getAssets(), "", destDir);
            marker.createNewFile();
        } catch (IOException e) {
            Log.e(TAG, "Failed to extract assets", e);
        }
        return destDir;
    }

    private void copyAssetDir(AssetManager am, String srcDir, String destDir) throws IOException {
        String[] items = am.list(srcDir);
        if (items == null) return;

        for (String item : items) {
            String srcPath = srcDir.isEmpty() ? item : srcDir + "/" + item;
            String destPath = destDir + "/" + (srcDir.isEmpty() ? item : srcDir + "/" + item);

            // Try to list as directory
            String[] children = am.list(srcPath);
            if (children != null && children.length > 0) {
                // It's a directory
                new File(destPath).mkdirs();
                // For subdirectories, we need to copy recursively but with flat destDir
                copyAssetDirRecursive(am, srcPath, destDir + "/" + srcPath + "/");
            } else {
                // It's a file
                File destFile = new File(destPath);
                destFile.getParentFile().mkdirs();
                copyAssetFile(am, srcPath, destFile);
            }
        }
    }

    private void copyAssetDirRecursive(AssetManager am, String srcDir, String destDir) throws IOException {
        String[] items = am.list(srcDir);
        if (items == null) return;

        new File(destDir).mkdirs();
        for (String item : items) {
            String srcPath = srcDir + "/" + item;
            String destPath = destDir + item;

            String[] children = am.list(srcPath);
            if (children != null && children.length > 0) {
                copyAssetDirRecursive(am, srcPath, destPath + "/");
            } else {
                copyAssetFile(am, srcPath, new File(destPath));
            }
        }
    }

    private void copyAssetFile(AssetManager am, String srcPath, File destFile) throws IOException {
        destFile.getParentFile().mkdirs();
        try (InputStream in = am.open(srcPath);
             OutputStream out = new FileOutputStream(destFile)) {
            byte[] buf = new byte[8192];
            int len;
            while ((len = in.read(buf)) > 0) {
                out.write(buf, 0, len);
            }
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        System.exit(0);
    }

    @Override
    protected String[] getLibraries() {
        return new String[] {
                "SDL2",
                "chiaki-borealis"
        };
    }

    @Override
    public void onBackPressed() {
        // Don't finish the activity. BACK is handled by borealis via SDL key events.
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        int keyCode = event.getKeyCode();
        int action = event.getAction();

        // Intercept DPAD and navigation keys to ensure they reach SDL as keyboard events
        // On Android TV, these may be consumed by SDLControllerManager as joystick events
        // before reaching the keyboard handler
        switch (keyCode) {
            case KeyEvent.KEYCODE_BACK:
                if (action == KeyEvent.ACTION_DOWN) onNativeKeyDown(KeyEvent.KEYCODE_BACK);
                else if (action == KeyEvent.ACTION_UP) onNativeKeyUp(KeyEvent.KEYCODE_BACK);
                return true;

            case KeyEvent.KEYCODE_DPAD_CENTER:
                // Remap to ENTER so SDL maps it to SDL_SCANCODE_RETURN (borealis BUTTON_A)
                if (action == KeyEvent.ACTION_DOWN) onNativeKeyDown(KeyEvent.KEYCODE_ENTER);
                else if (action == KeyEvent.ACTION_UP) onNativeKeyUp(KeyEvent.KEYCODE_ENTER);
                return true;

            case KeyEvent.KEYCODE_DPAD_UP:
            case KeyEvent.KEYCODE_DPAD_DOWN:
            case KeyEvent.KEYCODE_DPAD_LEFT:
            case KeyEvent.KEYCODE_DPAD_RIGHT:
                // Send DPAD as native key events directly
                if (action == KeyEvent.ACTION_DOWN) onNativeKeyDown(keyCode);
                else if (action == KeyEvent.ACTION_UP) onNativeKeyUp(keyCode);
                return true;
        }

        return super.dispatchKeyEvent(event);
    }
}
