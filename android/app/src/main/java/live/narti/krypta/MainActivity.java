// KRYPTA for Android: the web app (KRYPTA, VAULT, BUNKER) in a WebView, served from the APK's assets over
// https://appassets.androidplatform.net so AudioWorklet and the microphone work offline. Narti Live, MIT license.
package live.narti.krypta;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.media.AudioManager;
import android.net.Uri;
import android.os.Bundle;
import android.webkit.PermissionRequest;
import android.webkit.WebChromeClient;
import android.webkit.WebResourceRequest;
import android.webkit.WebResourceResponse;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import androidx.webkit.WebViewAssetLoader;

public class MainActivity extends Activity {
    private WebView web;
    private PermissionRequest pending;

    @Override
    protected void onCreate(Bundle state) {
        super.onCreate(state);
        setVolumeControlStream(AudioManager.STREAM_MUSIC);
        final WebViewAssetLoader loader = new WebViewAssetLoader.Builder()
                .addPathHandler("/", new WebViewAssetLoader.AssetsPathHandler(this)).build();
        web = new WebView(this);
        WebSettings s = web.getSettings();
        s.setJavaScriptEnabled(true);
        s.setDomStorageEnabled(true);
        s.setMediaPlaybackRequiresUserGesture(false);
        s.setAllowFileAccess(false);
        web.setBackgroundColor(0xff0a0a09);
        web.setWebViewClient(new WebViewClient() {
            @Override
            public WebResourceResponse shouldInterceptRequest(WebView v, WebResourceRequest r) {
                Uri u = r.getUrl();
                String p = u.getPath();
                if (p != null && p.endsWith("/")) u = u.buildUpon().path(p + "index.html").build();
                return loader.shouldInterceptRequest(u);
            }
        });
        web.setWebChromeClient(new WebChromeClient() {
            @Override
            public void onPermissionRequest(final PermissionRequest req) {
                if (checkSelfPermission(Manifest.permission.RECORD_AUDIO) == PackageManager.PERMISSION_GRANTED) {
                    req.grant(new String[] { PermissionRequest.RESOURCE_AUDIO_CAPTURE });
                } else {
                    pending = req;
                    requestPermissions(new String[] { Manifest.permission.RECORD_AUDIO }, 1);
                }
            }
        });
        setContentView(web);
        if (state != null) web.restoreState(state);
        else web.loadUrl("https://appassets.androidplatform.net/krypta/index.html");
    }

    @Override
    public void onRequestPermissionsResult(int code, String[] perms, int[] res) {
        if (pending == null) return;
        if (res.length > 0 && res[0] == PackageManager.PERMISSION_GRANTED) pending.grant(new String[] { PermissionRequest.RESOURCE_AUDIO_CAPTURE });
        else pending.deny();
        pending = null;
    }

    @Override
    protected void onSaveInstanceState(Bundle out) { super.onSaveInstanceState(out); web.saveState(out); }

    @Override
    public void onBackPressed() { if (web.canGoBack()) web.goBack(); else super.onBackPressed(); }
}
