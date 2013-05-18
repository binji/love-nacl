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

// The context menu event sends the URL to load after the tab is created.
function onChromeMessage(message, sender, sendResponse) {
  var url = message;
  createModule(url);
}
chrome.runtime.onMessage.addListener(onChromeMessage);
