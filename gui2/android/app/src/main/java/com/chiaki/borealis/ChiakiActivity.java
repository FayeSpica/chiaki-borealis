// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL

package com.chiaki.borealis;

import android.app.AlertDialog;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.widget.EditText;
import android.widget.FrameLayout;

import org.libsdl.app.SDL;
import org.libsdl.app.SDLActivity;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class ChiakiActivity extends SDLActivity {

    private static final String TAG = "chiaki-borealis";

    // Native callback for text input result
    public static native void nativeTextInputResult(String text, boolean submitted);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        String resPath = extractAssets();
        String configDir = getFilesDir().getAbsolutePath() + "/";

        for (String lib : getLibraries()) {
            SDL.loadLibrary(lib);
        }

        nativeSetenv("BOREALIS_RESOURCES", resPath);
        nativeSetenv("CHIAKI_CONFIG_DIR", configDir);

        Log.i(TAG, "ChiakiActivity created, resources at: " + resPath);

        super.onCreate(savedInstanceState);
    }

    /**
     * Called from native code to show a text input dialog.
     * Runs on UI thread, returns result via nativeTextInputResult callback.
     */
    public static void showTextInputDialog(String title, String initialText, int maxLength) {
        final SDLActivity activity = (SDLActivity) SDLActivity.getContext();
        activity.runOnUiThread(() -> {
            final EditText editText = new EditText(activity);
            editText.setText(initialText);
            editText.setSelection(initialText.length());
            editText.setHint(title);
            editText.setTextColor(0xFFFFFFFF);
            editText.setHintTextColor(0xFF888888);
            editText.setBackgroundColor(0xFF2D2D2D);
            editText.setPadding(32, 24, 32, 24);
            if (maxLength > 0) {
                editText.setFilters(new android.text.InputFilter[]{
                    new android.text.InputFilter.LengthFilter(maxLength)
                });
            }

            FrameLayout container = new FrameLayout(activity);
            container.setBackgroundColor(0xFF1A1A2E);
            container.setPadding(48, 32, 48, 32);
            FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.MATCH_PARENT,
                FrameLayout.LayoutParams.WRAP_CONTENT
            );
            editText.setLayoutParams(params);
            container.addView(editText);

            AlertDialog dialog = new AlertDialog.Builder(activity, android.R.style.Theme_DeviceDefault_Dialog_NoActionBar)
                .setView(container)
                .setPositiveButton("OK", (d, which) -> {
                    String result = editText.getText().toString();
                    nativeTextInputResult(result, true);
                })
                .setNegativeButton("Cancel", (d, which) -> {
                    nativeTextInputResult("", false);
                })
                .setOnCancelListener(d -> {
                    nativeTextInputResult("", false);
                })
                .create();

            dialog.getWindow().setBackgroundDrawableResource(android.R.color.transparent);
            dialog.show();

            // Style buttons to match dark theme
            dialog.getButton(AlertDialog.BUTTON_POSITIVE).setTextColor(0xFF00E68A);
            dialog.getButton(AlertDialog.BUTTON_NEGATIVE).setTextColor(0xFF888888);

            editText.requestFocus();
        });
    }

    private String extractAssets() {
        String destDir = getFilesDir().getAbsolutePath() + "/res/";
        File dest = new File(destDir);

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

            String[] children = am.list(srcPath);
            if (children != null && children.length > 0) {
                new File(destPath).mkdirs();
                copyAssetDirRecursive(am, srcPath, destDir + "/" + srcPath + "/");
            } else {
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
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        int keyCode = event.getKeyCode();
        int action = event.getAction();

        switch (keyCode) {
            case KeyEvent.KEYCODE_BACK:
                if (action == KeyEvent.ACTION_DOWN) onNativeKeyDown(KeyEvent.KEYCODE_BACK);
                else if (action == KeyEvent.ACTION_UP) onNativeKeyUp(KeyEvent.KEYCODE_BACK);
                return true;

            case KeyEvent.KEYCODE_DPAD_CENTER:
                if (action == KeyEvent.ACTION_DOWN) onNativeKeyDown(KeyEvent.KEYCODE_ENTER);
                else if (action == KeyEvent.ACTION_UP) onNativeKeyUp(KeyEvent.KEYCODE_ENTER);
                return true;

            case KeyEvent.KEYCODE_DPAD_UP:
            case KeyEvent.KEYCODE_DPAD_DOWN:
            case KeyEvent.KEYCODE_DPAD_LEFT:
            case KeyEvent.KEYCODE_DPAD_RIGHT:
                if (action == KeyEvent.ACTION_DOWN) onNativeKeyDown(keyCode);
                else if (action == KeyEvent.ACTION_UP) onNativeKeyUp(keyCode);
                return true;
        }

        return super.dispatchKeyEvent(event);
    }
}
