// KRYPTA offline cache: the app works without a network once opened.
var C = 'krypta-v2', FILES = ['./', 'index.html', 'engine.js?v=2', 'manifest.webmanifest', 'icon-192.png', 'icon-512.png',
  '../assets/favicon.svg', '../assets/fonts/BarlowCondensed-Medium.woff', '../assets/fonts/BarlowCondensed-SemiBold.woff', '../assets/fonts/BarlowCondensed-Bold.woff'];
self.addEventListener('install', function (e) { e.waitUntil(caches.open(C).then(function (c) { return c.addAll(FILES); })); self.skipWaiting(); });
self.addEventListener('activate', function (e) { e.waitUntil(caches.keys().then(function (ks) { return Promise.all(ks.filter(function (k) { return k !== C; }).map(function (k) { return caches.delete(k); })); })); });
self.addEventListener('fetch', function (e) {
  e.respondWith(fetch(e.request).then(function (r) { var cp = r.clone(); caches.open(C).then(function (c) { c.put(e.request, cp); }); return r; }).catch(function () { return caches.match(e.request); }));
});
