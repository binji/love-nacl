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

function getQueryParameters() {
  var result = {};
  var kvps = window.location.search.substr(1).split('&');
  for (var i = 0; i < kvps.length; ++i) {
    var kvp = kvps[i].split('=');
    result[decodeURIComponent(kvp[0])] = decodeURIComponent(kvp[1]);
  }
  return result;
}

document.addEventListener('DOMContentLoaded', function () {
  var queryParams = getQueryParameters();
  createModule(queryParams.url);
});
