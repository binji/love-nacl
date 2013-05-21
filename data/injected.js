(function () {

var bodyEl = document.body;
var embedEl = document.embeds[0];
var parentEl = embedEl.parentElement;
var messageEl;
var startTimeMs = Date.now();
var timeTillMessageMs = 1000;  // 1 second.

bodyEl.style.backgroundColor = '#000';


var windowMessages = (function() {
  var nextId = 0;
  var callbacks = {};

  function onWindowMessage(e) {
    if (e.source !== window)
      return;

    if (e.data.cmd !== 'response')
      return;

    var callback = callbacks[e.data.id];
    if (callback) {
      callback(e.data.data);
      if (e.data.finished)
        delete callbacks[e.data.id];
    }
  }

  function post(cmd, data, callback) {
    var id = nextId++;
    callbacks[id] = callback;
    window.postMessage({cmd: cmd, id: id, data: data}, '*');
  }

  window.addEventListener('message', onWindowMessage);

  return {
    post: post,
  };
})();


function onWindowResize(e) {
  var embedWidth = embedEl.clientWidth;
  var embedHeight = embedEl.clientHeight;
  var windowWidth = window.innerWidth;
  var windowHeight = window.innerHeight;
  // The embed element can only be centered programmatically -- if I try
  // to center it using CSS, I have to change its style which destroys
  // and recreates it.
  //
  // Strangely, not all style changes do this -- but adding "position:
  // absolute", or adding "display: -webkit-flex" to the parent does.
  embedEl.style.marginLeft = (windowWidth - embedWidth) / 2 + 'px';
  embedEl.style.marginTop = (windowHeight - embedHeight) / 2 + 'px';
}

// Function from underscore.js. See underscorejs.org
function debounce(func, wait, immediate) {
  var timeout;
  return function() {
    var context = this, args = arguments;
    var later = function() {
      timeout = null;
      if (!immediate) func.apply(context, args);
    };
    var callNow = immediate && !timeout;
    clearTimeout(timeout);
    timeout = setTimeout(later, wait);
    if (callNow) func.apply(context, args);
  };
};

function centerElement(el) {
  el.style.bottom = '0';
  el.style.left = '0';
  el.style.margin = 'auto';
  el.style.position = 'absolute';
  el.style.right = '0';
  el.style.top = '0';
}

function makeMessageEl() {
  if (!messageEl) {
    messageEl = document.createElement('div');
    messageEl.style.color = '#fff';
    messageEl.style.fontSize = '30px';
    messageEl.style.height = '100px';
    messageEl.style.textAlign = 'center';
    messageEl.style.width = '500px';
    centerElement(messageEl);
    parentEl.appendChild(messageEl);
  }
}

function setMessage(message) {
  var elapsed = Date.now() - startTimeMs;
  if (elapsed < timeTillMessageMs)
    return;

  makeMessageEl();
  messageEl.textContent = message;
  console.log(message);
}

function onModuleLoadStart(e) {
  setMessage('loading nexe...');
}

function onModuleLoad(e) {
  setMessage('loading nexe 100%');
}

function onModuleProgress(e) {
  if (e.lengthComputable) {
    setMessage('loading nexe '+(e.loaded / e.total * 100).toFixed(0)+'%');
  } else {
    setMessage('loading nexe...');
  }
}

function onModuleError(e) {
  setMessage('error. :( "'+embedEl.lastError+'"');
}

function onModuleCrash(e) {
  setMessage('crash. :(');
}

function onMessageSetWindow(width, height) {
  embedEl.width = parseInt(width, 10) + 'px';
  embedEl.height = parseInt(height, 10) + 'px';
  onWindowResize();
}

function humanReadable(size) {
  var K = 1024;
  var M = 1024 * K;
  var G = 1024 * M;
  if (size > G) {
    return (size / G).toFixed(1) + 'G';
  } else if (size > M) {
    return (size / M).toFixed(1) + 'M';
  } else if (size > K) {
    return (size / K).toFixed(1) + 'K';
  } else {
    return size + 'B';
  }
}

function onMessageDownload(downloaded, max) {
  downloaded = parseInt(downloaded, 10);
  max = parseInt(max, 10);

  if (max) {
    setMessage('downloading '+(downloaded / max * 100).toFixed(0)+'%');
  } else {
    setMessage('downloaded '+humanReadable(downloaded));
  }
}

function onMessageOK() {
  function afterPause() {
    embedEl.style.maxHeight = '';
    onWindowResize();
    if (messageEl)
      parentEl.removeChild(messageEl);
    messageEl = undefined;
  }

  function notifyModule() {
    embedEl.postMessage('run');
    setMessage('done!');
    setTimeout(afterPause, 200);
  }

  function onGetFiles(data) {
    if (data.done) {
      // Done!
      notifyModule();
    } else {
      if (data.type === 'file') {
        embedEl.postMessage('copyFile:'+data.path);
      } else if (data.type === 'dir') {
        embedEl.postMessage('makeDir:'+data.path);
      }
    }
  }

  function onQueryFileSystem(data) {
    if (data.ok) {
      // Persistent storage. Request filenames...
      embedEl.postMessage('fileSystemAccess:yes');
      windowMessages.post('getFiles', {}, onGetFiles);
    } else {
      // No persistent storage, so just run!
      notifyModule();
    }
  }

  // HACK(binji): Check filesystem to see if files need to be copied...
  setMessage('checking filesystem...');
  windowMessages.post('queryFileSystem', {}, onQueryFileSystem);
}

function onMessageRequestFileSystem() {
  function onRequestFileSystem(data) {
    var ok = data.ok ? 'yes' : 'no';
    embedEl.postMessage('fileSystemAccess:'+ok);
  }

  windowMessages.post('requestFileSystem', {}, onRequestFileSystem);
}

function onMessageBye() {
  // User is quitting.
  parentEl.removeChild(embedEl);

  function createLinkButton(text) {
    var el = document.createElement('a');
    el.setAttribute('href', '#');
    el.style.color = '#fff';
    el.style.display = 'block';
    el.style.fontSize = '30px';
    el.style.textAlign = 'center';
    el.textContent = text;
    return el;
  }

  var buttonsEl = document.createElement('div');
  buttonsEl.style.height = '100px';
  buttonsEl.style.width = '500px';
  centerElement(buttonsEl);
  var backText = window.history.length > 1 ? 'back' : 'close';
  var backButtonEl = createLinkButton(backText);
  var reloadButtonEl = createLinkButton('reload');
  buttonsEl.appendChild(backButtonEl);
  buttonsEl.appendChild(reloadButtonEl);
  parentEl.appendChild(buttonsEl);

  backButtonEl.addEventListener('click', function (e) {
    if (window.history.length > 1)
      windowMessages.post('back');
    else
      windowMessages.post('close');
    e.preventDefault();
  });

  reloadButtonEl.addEventListener('click', function (e) {
    windowMessages.post('reload');
    e.preventDefault();
  });
}

var messageMap = {
  'setWindow': onMessageSetWindow,
  'download': onMessageDownload,
  'OK': onMessageOK,
  'requestFileSystem': onMessageRequestFileSystem,
  'bye': onMessageBye,
};

function onModuleMessage(e) {
  if (typeof e.data !== 'string')
    return;

  var msg = e.data;
  console.log('got module message: '+msg);
  var parts = msg.split(':');
  var cmd = parts[0];
  var values = parts[1] ? parts[1].split(',') : [];
  var func = messageMap[cmd];
  if (func) {
    if (values.length === func.length) {
      func.apply(null, values);
    } else {
      console.log('Expected '+func.length+' arguments to module command "'+
          cmd+'", Got '+values.length+'.');
    }
  } else {
    console.log('Unknown module command: '+cmd);
  }
}

function injectExtensionScript(name) {
  // Inject filesystem script to access filesystem API.  Idea copied from
  // http://stackoverflow.com/questions/9515704/building-a-chrome-extension-inject-code-in-a-page-using-a-content-script/9517879#9517879
  var scriptEl = document.createElement('script');
  scriptEl.src = 'chrome-extension://mompnkcmpbopandjnddeecgeeojegohc/' + name;
  scriptEl.addEventListener('load', function () {
    this.parentNode.removeChild(this);
  });
  document.documentElement.appendChild(scriptEl);
}

embedEl.style.maxHeight = '0';

parentEl.addEventListener('loadstart', onModuleLoadStart, true);
parentEl.addEventListener('progress', onModuleProgress, true);
parentEl.addEventListener('load', onModuleLoad, true);
parentEl.addEventListener('crash', onModuleCrash, true);
parentEl.addEventListener('error', onModuleError, true);
parentEl.addEventListener('message', onModuleMessage, true);

window.addEventListener('resize', debounce(onWindowResize, 100));

injectExtensionScript('filesystem.js');

})();
