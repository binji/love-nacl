var bodyEl = document.body;
var embedEl = document.embeds[0];
var parentEl = embedEl.parentElement;
var messageEl;
var startTimeMs = Date.now();
var timeTillMessageMs = 1000;  // 1 second.

bodyEl.style.backgroundColor = '#000';

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

function onLoadStart(e) {
  setMessage('loading nexe...');
}

function onLoad(e) {
  setMessage('loading nexe 100%');
}

function onProgress(e) {
  if (e.lengthComputable) {
    setMessage('loading nexe '+(e.loaded / e.total * 100).toFixed(0)+'%');
  } else {
    setMessage('loading nexe...');
  }
}

function onError(e) {
  setMessage('error. :( "'+embedEl.lastError+'"');
}

function onCrash(e) {
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
    console.log('OK!');
  }

  setMessage('done!');
  setTimeout(afterPause, 200);
}

var messageMap = {
  'setWindow': onMessageSetWindow,
  'download': onMessageDownload,
  'OK': onMessageOK,
};

function onMessage(e) {
  if (typeof e.data !== 'string')
    return;

  var msg = e.data;
  console.log('got message: '+msg);
  for (var key in messageMap) {
    if (messageMap.hasOwnProperty(key)) {
      if (msg.lastIndexOf(key, 0) === 0) {
        // Extract values.
        var values = msg.split(':')[1];
        var numValues;
        if (values) {
          values = values.split(',');
          numValues = values.length;
        } else {
          numValues = 0;
        }

        var func = messageMap[key];
        if (numValues === func.length)
          func.apply(null, values);
        break;
      }
    }
  }
}

embedEl.style.maxHeight = '0';

parentEl.addEventListener('loadstart', onLoadStart, true);
parentEl.addEventListener('progress', onProgress, true);
parentEl.addEventListener('load', onLoad, true);
parentEl.addEventListener('crash', onCrash, true);
parentEl.addEventListener('error', onError, true);
parentEl.addEventListener('message', onMessage, true);

window.addEventListener('resize', debounce(onWindowResize, 100));
