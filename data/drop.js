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

  function onTransitionEnd(e) {
    document.body.removeEventListener('transitionend', onTransitionEnd, true);
    createModule(url);
  }

  document.body.addEventListener('transitionend', onTransitionEnd, true);
  document.body.classList.add('loading');

  e.stopPropagation();
  e.preventDefault();
}

function createModule(url) {
  // Get the manifest to find the correct nmf to use.
  var manifest = chrome.runtime.getManifest();
  var nmf = manifest.nacl_modules[0].path;

  var embedEl = document.createElement('embed');
  embedEl.setAttribute('src', nmf);
  embedEl.setAttribute('type', 'application/x-nacl');
  embedEl.setAttribute('love_src', url);
  document.body.appendChild(embedEl);

  // Inject code the same way as if this were a mimetype handler, to share
  // code.  Idea copied from
  // http://stackoverflow.com/questions/9515704/building-a-chrome-extension-inject-code-in-a-page-using-a-content-script/9517879#9517879
  var scriptEl = document.createElement('script');
  scriptEl.src = chrome.extension.getURL('injected.js');
  scriptEl.addEventListener('load', function () {
    this.parentNode.removeChild(this);
  });
  document.head.appendChild(scriptEl);
}

document.addEventListener('DOMContentLoaded', function() {
  $('#dropTarget').addEventListener('dragleave', onDragLeave, false);
  $('#dropTarget').addEventListener('dragover', onDragOver, false);
  $('#dropTarget').addEventListener('drop', onDrop, false);
});
