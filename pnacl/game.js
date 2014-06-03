var baseUrl = 'https://storage.googleapis.com/loveliness/';

function createModule(url) {
  var nmf = baseUrl + 'pexe/2.4-2c44f9a/love_release.nmf';
  var embedEl = document.createElement('embed');
  embedEl.setAttribute('src', nmf);
  embedEl.setAttribute('type', 'application/x-pnacl');
  embedEl.setAttribute('love_src', url);
  document.body.appendChild(embedEl);

  var scriptEl = document.createElement('script');
  scriptEl.src = 'injected.js';
  scriptEl.addEventListener('load', function () {
    this.parentNode.removeChild(this);
  });
  document.head.appendChild(scriptEl);
}

var loves = {
  'mari0': 'mari0_1.6.love',
  'iyfct': 'iyfct.love',
  'robot': 'Ortho%20Robot.love',
  'metaballs': 'metaballs-fixed.love',
  'no': 'no.love',
};

var locUrl = window.location.href;
var lastSlash = locUrl.lastIndexOf('/');
var name = locUrl.slice(lastSlash + 1);
var firstDot = name.indexOf('.');
name = name.slice(0, firstDot);
var loveUrl = baseUrl + 'love/' + loves[name];

document.title = name + ' - ' + document.title;

createModule(loveUrl);
