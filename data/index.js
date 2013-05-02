function $(selector) {
  return document.querySelector(selector);
}

function onDragOver(e) {
  e.dataTransfer.dropEffect = 'copy';
  $('#dropTarget').classList.add('dragover');
  e.stopPropagation();
  e.preventDefault();
}

function onDragLeave(e) {
  $('#dropTarget').classList.remove('dragover');
  e.stopPropagation();
  e.preventDefault();
}

function onDrop(e) {
  var file = e.dataTransfer.files[0];
  var url = URL.createObjectURL(file);
  document.body.removeChild($('#dropTarget'));

  function onLoad(e) {
  }

  function onMessage(e) {
    if (typeof e.data !== 'string')
      return;
    var msg = e.data;
    if (msg === 'OK') {
      // loaded successfully.
      URL.revokeObjectURL(url);
    }
  }

  function createModule() {
    var listenerEl = document.createElement('div');
    listenerEl.setAttribute('id', 'listener');
    listenerEl.classList.add('fullscreen');
    listenerEl.addEventListener('load', onLoad, true);
    listenerEl.addEventListener('message', onMessage, true);

    var moduleEl = document.createElement('embed');
    moduleEl.setAttribute('src', 'love_release.nmf');
    moduleEl.setAttribute('type', 'application/x-nacl');
    moduleEl.setAttribute('love_src', url);

    listenerEl.appendChild(moduleEl);
    document.body.appendChild(listenerEl);
  }

  function onTransitionEnd(e) {
    document.body.removeEventListener('transitionend', onTransitionEnd, true);
    createModule();
  }

  document.body.addEventListener('transitionend', onTransitionEnd, true);
  document.body.classList.add('loading');

  e.stopPropagation();
  e.preventDefault();
}

document.addEventListener('DOMContentLoaded', function() {
  $('#dropTarget').addEventListener('dragleave', onDragLeave, false);
  $('#dropTarget').addEventListener('dragover', onDragOver, false);
  $('#dropTarget').addEventListener('drop', onDrop, false);
});
