document.addEventListener('DOMContentLoaded', function() {
  window.webkitStorageInfo.requestQuota(window.PERSISTENT, 1024*1024,
      function(bytes) {
        console.log(
            'Allocated '+bytes+' bytes of persistant storage.');
        createNaClModule();
        attachDefaultListeners();
      },
      function(e) { alert('Failed to allocate space') });
});

function createNaClModule() {
  var moduleEl = document.createElement('embed');
  moduleEl.setAttribute('src', 'love_debug.nmf');
  moduleEl.setAttribute('type', 'application/x-nacl');
  moduleEl.setAttribute('love_src', 'mari0_1.6.love');

  var listenerDiv = document.getElementById('listener');
  listenerDiv.appendChild(moduleEl);
}

function attachDefaultListeners() {
  var listenerDiv = document.getElementById('listener');
  /*
  listenerDiv.addEventListener('load', moduleDidLoad, true);
  listenerDiv.addEventListener('message', handleMessage, true);

  if (typeof window.attachListeners !== 'undefined') {
    window.attachListeners();
  }
  */
}
