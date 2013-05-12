function onLaunched(launchData) {
  chrome.app.window.create('index.html', {
    width: {{width}},
    height: {{height}}
  });
}

chrome.app.runtime.onLaunched.addListener(onLaunched);
