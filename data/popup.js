'use strict';

var global = this;
var port = null;

function initializePort(port) {
  global.port = port;
  port.onMessage.addListener(onPortMessage);
  postMessage('popupConnected');
}

function postMessage(cmd, data) {
  if (!port) {
    console.log('Error posting message, port not available!');
    return;
  }

  port.postMessage({
    cmd: cmd,
    data: data,
    sender: 'popup'
  });
}

function setSettings(newSettings) {
  postMessage('setSettings', newSettings);
}

function onPortMessage(message) {
  console.log('onMessage called with message: ' + JSON.stringify(message));
  if (message.sender === 'popup')
    return;

  if (message.cmd === 'setSettings') {
    var newSettings = message.data;

    for (var key in newSettings) {
      var value = newSettings[key];
      var changedHandlerName = 'onSettingChanged_' + key;
      var fn = global[changedHandlerName];
      if (typeof fn === 'function')
        fn(value);
    }
  }
}

function onSettingChanged_enabled(enabled) {
  var enabledMessage = '<span class="enabled">On</span>, and ready to LÖVE!';
  var disabledMessage =
      '<span class="disabled">Off</span>... no LÖVE for me...';

  $('#enableStatus').innerHTML = enabled ?  enabledMessage : disabledMessage;
  $('#enableLink').textContent = enabled ? 'disable' : 'enable';
}

function onSettingChanged_textColor(color) {
  $('#colorLabel').style.color = color;
}

function onSettingChanged_backgroundColor(color) {
  $('#colorLabel').style.backgroundColor = color;
}


// Handle events from the page.
function $(selector) {
  return document.querySelector(selector);
}

function onEnableClicked(e) {
  postMessage('toggleEnabled');
}

function onTestClicked(e) {
  chrome.tabs.create({url: 'test.html'});
  window.close();
}

function onLoadFileClicked(e) {
  // Load drag and drop page.
  chrome.tabs.create({url: 'drop.html'});
  window.close();
}

function getColorClass(el) {
  if (el.classList.length < 2)
    return 'color-black';

  for (var i = 0; i < el.classList.length; ++i) {
    if (el.classList[i].lastIndexOf('color-', 0) === 0) {
      return el.classList[i];
    }
  }

  // Default to black.
  return 'color-black';
}

function onColorClicked(e) {
  var style = window.getComputedStyle(e.target);
  var textColor = style.color;
  var bgColor = style.backgroundColor;

  setSettings({
    textColor: style.color,
    backgroundColor: style.backgroundColor
  });
}

document.addEventListener('DOMContentLoaded', function() {
  initializePort(chrome.runtime.connect({name: 'popup'}));

  $('#enableLink').addEventListener('click', onEnableClicked, false);
  $('#testLink').addEventListener('click', onTestClicked, false);
  $('#loadFileLink').addEventListener('click', onLoadFileClicked, false);

  var swatches = document.querySelectorAll('.swatch');
  for (var i = 0; i < swatches.length; ++i) {
    swatches[i].addEventListener('click', onColorClicked, false);
  }
});
