// VnKey's introduction page: the latest release and the typing demo. Shared by both languages.
(function () {
  "use strict";
  var REPO = "minhkhoi-dev/vnkey-s60-eseries";

  // The newest release: its version and a direct link to its .sisx.
  if (window.fetch) {
    fetch("https://api.github.com/repos/" + REPO + "/releases/latest", { headers: { Accept: "application/vnd.github+json" } })
      .then(function (r) { return r.ok ? r.json() : null; })
      .then(function (rel) {
        if (!rel || !rel.tag_name) return;
        var version = rel.tag_name.replace(/^v/, "");
        var sisx = (rel.assets || []).filter(function (a) { return /\.sisx$/i.test(a.name); })[0];
        document.querySelectorAll(".js-ver").forEach(function (el) { el.textContent = (el.getAttribute("data-fmt") || "%s").replace("%s", version); });
        if (sisx) {
          document.querySelectorAll(".js-dl").forEach(function (el) { el.href = sisx.browser_download_url; });
          document.querySelectorAll(".js-file").forEach(function (el) { el.textContent = sisx.name; });
        }
      })
      .catch(function () {});
  }

  // The [V] mark, drawn as the phone draws it: a 5 x 7 letter inside an 11 x 13 frame.
  var V = ["X...X", "X...X", "X...X", "X...X", ".X.X.", ".X.X.", "..X.."];
  var d = "";
  V.forEach(function (row, y) {
    for (var x = 0; x < 5; x++) if (row[x] === "X") d += "M" + (3 + x) + " " + (3 + y) + "h1v1h-1z";
  });
  document.getElementById("markLetter").setAttribute("d", d);

  var clock = document.getElementById("clock");
  function tick() {
    var t = new Date();
    clock.textContent = ("0" + t.getHours()).slice(-2) + ":" + ("0" + t.getMinutes()).slice(-2);
  }
  tick();
  setInterval(tick, 15000);

  // The typing demo. Each message has the keys pressed and, for every key, the text the
  // editor holds after it, as VnKey's Telex engine (symbian/src/VnTelex.cpp) gives it.
  var DEMO = [{"keys":"Chaof banj, minhf ddang gox tieengs Vieejt treen Nokia E72, ko caanf mays tinhs.","frames":["C","Ch","Cha","Chao","Chào","Chào ","Chào b","Chào ba","Chào ban","Chào bạn","Chào bạn,","Chào bạn, ","Chào bạn, m","Chào bạn, mi","Chào bạn, min","Chào bạn, minh","Chào bạn, mình","Chào bạn, mình ","Chào bạn, mình d","Chào bạn, mình đ","Chào bạn, mình đa","Chào bạn, mình đan","Chào bạn, mình đang","Chào bạn, mình đang ","Chào bạn, mình đang g","Chào bạn, mình đang go","Chào bạn, mình đang gõ","Chào bạn, mình đang gõ ","Chào bạn, mình đang gõ t","Chào bạn, mình đang gõ ti","Chào bạn, mình đang gõ tie","Chào bạn, mình đang gõ tiê","Chào bạn, mình đang gõ tiên","Chào bạn, mình đang gõ tiêng","Chào bạn, mình đang gõ tiếng","Chào bạn, mình đang gõ tiếng ","Chào bạn, mình đang gõ tiếng V","Chào bạn, mình đang gõ tiếng Vi","Chào bạn, mình đang gõ tiếng Vie","Chào bạn, mình đang gõ tiếng Viê","Chào bạn, mình đang gõ tiếng Việ","Chào bạn, mình đang gõ tiếng Việt","Chào bạn, mình đang gõ tiếng Việt ","Chào bạn, mình đang gõ tiếng Việt t","Chào bạn, mình đang gõ tiếng Việt tr","Chào bạn, mình đang gõ tiếng Việt tre","Chào bạn, mình đang gõ tiếng Việt trê","Chào bạn, mình đang gõ tiếng Việt trên","Chào bạn, mình đang gõ tiếng Việt trên ","Chào bạn, mình đang gõ tiếng Việt trên N","Chào bạn, mình đang gõ tiếng Việt trên No","Chào bạn, mình đang gõ tiếng Việt trên Nok","Chào bạn, mình đang gõ tiếng Việt trên Noki","Chào bạn, mình đang gõ tiếng Việt trên Nokia","Chào bạn, mình đang gõ tiếng Việt trên Nokia ","Chào bạn, mình đang gõ tiếng Việt trên Nokia E","Chào bạn, mình đang gõ tiếng Việt trên Nokia E7","Chào bạn, mình đang gõ tiếng Việt trên Nokia E72","Chào bạn, mình đang gõ tiếng Việt trên Nokia E72,","Chào bạn, mình đang gõ tiếng Việt trên Nokia E72, ","Chào bạn, mình đang gõ tiếng Việt trên Nokia E72, k","Chào bạn, mình đang gõ tiếng Việt trên Nokia E72, ko","Chào bạn, mình đang gõ tiếng Việt trên Nokia E72, không ","Chào bạn, mình đang gõ tiếng Việt trên Nokia E72, không c","Chào bạn, mình đang gõ tiếng Việt trên Nokia E72, không ca","Chào bạn, mình đang gõ tiếng Việt trên Nokia E72, không câ","Chào bạn, mình đang gõ tiếng Việt trên Nokia E72, không cân","Chào bạn, mình đang gõ tiếng Việt trên Nokia E72, không cần","Chào bạn, mình đang gõ tiếng Việt trên Nokia E72, không cần ","Chào bạn, mình đang gõ tiếng Việt trên Nokia E72, không cần m","Chào bạn, mình đang gõ tiếng Việt trên Nokia E72, không cần ma","Chào bạn, mình đang gõ tiếng Việt trên Nokia E72, không cần may","Chào bạn, mình đang gõ tiếng Việt trên Nokia E72, không cần máy","Chào bạn, mình đang gõ tiếng Việt trên Nokia E72, không cần máy ","Chào bạn, mình đang gõ tiếng Việt trên Nokia E72, không cần máy t","Chào bạn, mình đang gõ tiếng Việt trên Nokia E72, không cần máy ti","Chào bạn, mình đang gõ tiếng Việt trên Nokia E72, không cần máy tin","Chào bạn, mình đang gõ tiếng Việt trên Nokia E72, không cần máy tinh","Chào bạn, mình đang gõ tiếng Việt trên Nokia E72, không cần máy tính","Chào bạn, mình đang gõ tiếng Việt trên Nokia E72, không cần máy tính."]},{"keys":"Tooi gwir file text qua email nhes.","frames":["T","To","Tô","Tôi","Tôi ","Tôi g","Tôi gư","Tôi gưi","Tôi gửi","Tôi gửi ","Tôi gửi f","Tôi gửi fi","Tôi gửi fil","Tôi gửi file","Tôi gửi file ","Tôi gửi file t","Tôi gửi file te","Tôi gửi file tẽ","Tôi gửi file tẽt","Tôi gửi file text ","Tôi gửi file text q","Tôi gửi file text qu","Tôi gửi file text qua","Tôi gửi file text qua ","Tôi gửi file text qua e","Tôi gửi file text qua em","Tôi gửi file text qua ema","Tôi gửi file text qua emai","Tôi gửi file text qua email","Tôi gửi file text qua email ","Tôi gửi file text qua email n","Tôi gửi file text qua email nh","Tôi gửi file text qua email nhe","Tôi gửi file text qua email nhé","Tôi gửi file text qua email nhé."]}];
  if (!DEMO || window.matchMedia("(prefers-reduced-motion: reduce)").matches) return;

  var typed = document.getElementById("typed");
  var msg = document.getElementById("msg");
  var capKeys = document.getElementById("capKeys");
  var capWord = document.getElementById("capWord");
  var keys = {};
  document.querySelectorAll("#kb .key").forEach(function (el) { keys[el.getAttribute("data-k")] = el; });
  var FN = { "1": "r", "2": "t", "3": "y", "4": "f", "5": "g", "6": "h", "7": "v", "8": "b", "9": "n", "0": "m" };

  function press(ch) {
    var list = [];
    if (/[a-z]/.test(ch)) list = [ch];
    else if (/[A-Z]/.test(ch)) list = ["shift", ch.toLowerCase()];
    else if (FN[ch]) list = ["fn", FN[ch]];
    else if (keys[ch]) list = [ch];
    list.forEach(function (k) {
      var el = keys[k];
      if (!el) return;
      el.classList.add("on");
      setTimeout(function () { el.classList.remove("on"); }, 150);
    });
  }

  var isLetter = /[A-Za-z]/;
  var wordEnd = /[\u00C0-\u1EF9A-Za-z]+$/;
  // The caption: the keys of the word being typed (or just ended) and what it became.
  function caption(m, i) {
    var end = isLetter.test(m.keys[i]) ? i + 1 : i;
    var start = end;
    while (start > 0 && isLetter.test(m.keys[start - 1])) start--;
    if (start === end) return;
    var text = isLetter.test(m.keys[i]) ? m.frames[i] : m.frames[i].slice(0, -1);
    var w = text.match(wordEnd);
    capKeys.textContent = m.keys.slice(start, end);
    capWord.textContent = w ? w[0] : "";
  }

  var visible = true;
  if ("IntersectionObserver" in window) {
    new IntersectionObserver(function (e) { visible = e[0].isIntersecting; }).observe(msg);
  }
  function wait(ms) {
    return new Promise(function (done) {
      (function check() {
        setTimeout(function () { (visible && !document.hidden) ? done() : check(); }, ms);
        ms = 250;
      })();
    });
  }

  async function play() {
    await wait(1600);
    for (;;) {
      for (var n = 0; n < DEMO.length; n++) {
        var m = DEMO[n];
        msg.classList.add("away");
        await wait(380);
        typed.textContent = "";
        msg.classList.remove("away");
        await wait(500);
        for (var i = 0; i < m.keys.length; i++) {
          var ch = m.keys[i];
          press(ch);
          typed.textContent = m.frames[i];
          caption(m, i);
          await wait(ch === " " ? 190 : 95 + Math.random() * 70);
        }
        await wait(3000);
      }
    }
  }
  play();
})();
