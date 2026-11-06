# KRYPTA for Android

A small WebView app that carries the KRYPTA web app (and VAULT and BUNKER) offline. GitHub Actions builds it
(`.github/workflows/krypta-apk.yml`) and commits the APK to `downloads/KRYPTA-v1.0.apk`.

The APK is signed with the runner's throw-away debug key, so Android asks to allow installs from this source,
and a later build may need the old app uninstalled first. No signing keys live in this repository.
