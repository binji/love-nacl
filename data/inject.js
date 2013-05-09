if (document.embeds[0] &&
    document.embeds[0].name === 'plugin' &&
    document.embeds[0].type === 'application/x-love-game') {
  if (!document.body.getAttribute('application_x_love_game_injected')) {
    document.body.setAttribute('application_x_love_game_injected', 'true');
    console.log('injected!');
    (function() {
      var bodyEl = document.body;
      var embedEl = document.embeds[0];
      var parentEl = embedEl.parentElement;

      bodyEl.style.backgroundColor = '#000';

      function onLoad(e) {
        embedEl.style.position = 'absolute';
        embedEl.style.top = '0';
        embedEl.style.bottom = '0';
        embedEl.style.left = '0';
        embedEl.style.right = '0';
        embedEl.style.margin = 'auto';
      }

      function onMessage(e) {
        if (typeof e.data !== 'string')
          return;

        var msg = e.data;
        var setWindowMsg = 'setWindow:';
        if (msg.lastIndexOf(setWindowMsg, 0) === 0) {
          var value = msg.substr(setWindowMsg.length);
          var comma = value.indexOf(',');
          if (comma >= 0) {
            var w = parseInt(value.substr(0, comma), 10);
            var h = parseInt(value.substr(comma + 1), 10);
            embedEl.width = w + 'px';
            embedEl.height = h + 'px';
          }
        }
      }

      parentEl.addEventListener('load', onLoad, true);
      parentEl.addEventListener('message', onMessage, true);
      if (embedEl.readyState !== undefined) {
        onLoad();
      }
    })();
  }
}
