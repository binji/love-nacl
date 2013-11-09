"use strict";

function $(selector) {
  return document.querySelector(selector);
}

var testIx = 0;
var tests = [
  { message: 'Looking for NaCl plugin', test: runFindNaClPlugin },
  { message: 'Trying NaCl plugin', test: runTryNaClPlugin },
  { message: 'Trying WebGL', test: runTryWebGL },
  { message: 'Trying LÖVELINESS', test: runTryLoveliness },
];

function nextTest() {
  if (testIx >= tests.length) {
    allTestsPassed();
    return;
  }

  var test = tests[testIx++];
  var stepsEl = $('#steps');
  var trEl = document.createElement('tr');
  var tdMessageEl = document.createElement('td');
  var tdCheckEl = document.createElement('td');
  tdMessageEl.textContent = test.message;
  trEl.appendChild(tdMessageEl);
  trEl.appendChild(tdCheckEl);
  stepsEl.appendChild(trEl);
  test.test(function (result, error) {
    if (result) {
      tdCheckEl.textContent = '\u2713';  // check mark
      trEl.classList.add('success');
      nextTest();
    } else {
      tdCheckEl.textContent = '\u2715';  // X mark
      trEl.classList.add('error');
      testFailed(error);
    }
  });
}

function runFindNaClPlugin(cb) {
  function getPluginByName(name) {
    for (var i = 0; i < navigator.plugins.length; ++i) {
      var plugin = navigator.plugins[i];
      if (plugin.name === name)
        return plugin;
    }
    return undefined;
  }

  function getPluginMimetype(plugin, mimetype) {
    var ix = 0;
    while (plugin[ix]) {
      var pluginMimetype = plugin[ix];
      if (pluginMimetype.type === mimetype)
        return pluginMimetype;
      ix++;
    }
    return undefined;
  }

  var plugin = getPluginByName('Native Client');
  if (!plugin)
    return cb(false, 'No Native Client plugin found!');

  var mimetype = getPluginMimetype(plugin, 'application/x-love-game');
  if (!mimetype)
    return cb(false, 'Native Client plugin doesn\'t support LÖVE?');

  return cb(true);
}

function runTryModule(cb, attrs) {
  var giveUpTime = 3 * 1000;  // 3 seconds.
  var workspaceEl = $('#workspace');
  workspaceEl.addEventListener('loadstart', moduleLoadStart, true);
  workspaceEl.addEventListener('load', moduleLoad, true);
  workspaceEl.addEventListener('error', moduleError, true);
  workspaceEl.addEventListener('message', moduleMessage, true);
  workspaceEl.addEventListener('crash', moduleCrash, true);

  var embedEl = document.createElement('embed');
  embedEl.setAttribute('type', 'application/x-nacl');
  for (var key in attrs) {
    embedEl.setAttribute(key, attrs[key]);
  }
  workspaceEl.appendChild(embedEl);
  var giveUpId = window.setTimeout(giveUp, giveUpTime);

  var moduleLoadStart = false;
  var moduleLoaded = false;

  function moduleLoadStart(e) {
    moduleLoadStart = true;
  }

  function moduleLoad(e) {
    moduleLoaded = true;
    embedEl.postMessage('OK?');
  }

  function moduleError(e) {
    window.clearInterval(giveUpId);
    cleanUpAndCallback(false, 'NaCl Module load error: ' + embedEl.lastError);
  }

  function moduleMessage(e) {
    if (typeof e.data !== 'string') {
      return cleanUpAndCallback(false, 'Received bad message...?');
    }
    if (e.data.lastIndexOf('download', 0) === 0) {
      // Ignore download messages.
      return;
    }
    if (e.data !== 'OK') {
      return cleanUpAndCallback(false, 'Received incorrect message...?');
    }
    cleanUpAndCallback(true);
  }

  function moduleCrash(e) {
    window.clearInterval(giveUpId);
    cleanUpAndCallback(
        false,
        'NaCl Module crashed. exit code: ' + embedEl.exitStatus);
  }

  function giveUp() {
    if (moduleLoaded) {
      cleanUpAndCallback(false, 'Module loaded, but no message received...');
    } else if (moduleLoadStart) {
      cleanUpAndCallback(
          false,
          'NaCl module started loading, but didn\'t finish.');
    } else {
      cleanUpAndCallback(false, 'NaCl module never started loading.');
    }
  }

  function cleanUpAndCallback(result, message) {
    workspaceEl.removeEventListener('loadstart', moduleLoadStart, true);
    workspaceEl.removeEventListener('load', moduleLoad, true);
    workspaceEl.removeEventListener('error', moduleError, true);
    workspaceEl.removeEventListener('message', moduleMessage, true);
    workspaceEl.removeEventListener('crash', moduleCrash, true);
    workspaceEl.removeChild(embedEl);
    cb(result, message);
  }
}

function runTryNaClPlugin(cb) {
  runTryModule(cb, {
    src: 'test_release.nmf',
    width: '200px',
    height: '200px',
  });
}

function runTryWebGL(cb) {
  var canvasEl = document.createElement('canvas');
  try {
    var names = ['webgl', 'experimental-webgl'];
    for (var i = 0; i < names.length; ++i) {
      var context = canvasEl.getContext(names[i]);
      if (context)
        return cb(true);
    }

    cb(false, 'Couldn\'t get a WebGL context from canvas.');
  } catch (error) {
    cb(false, 'Exception thrown getting WebGL context from canvas!');
  }
}

function runTryLoveliness(cb) {
  runTryModule(cb, {
    src: 'love_release.nmf',
    love_src: 'bogus',
    width: '800px',
    height: '600px',
  });
}

function allTestsPassed() {
  $('#alertExamples').addEventListener('click', function () {
    window.location.href = 'http://binji.github.io/love-nacl';
  });
  $('#alertClose').addEventListener('click', function () {
    window.close();
  });
  $('#alertSuccess').removeAttribute('hidden');
}

function testFailed(error) {
  $('#alertFileIssue').addEventListener('click', function () {
    chrome.tabs.create({url: 'https://github.com/binji/love-nacl/issues'});
  });
  var version = chrome.runtime.getManifest().version;
  $('#alertVersionMsg').textContent = 'LÖVELINESS version ' + version;
  $('#alertErrorMsg').textContent = error;
  $('#alertError').removeAttribute('hidden');
}

document.addEventListener('DOMContentLoaded', function () {
  nextTest();
});
