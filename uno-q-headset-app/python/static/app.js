async function api(path, options = {}) {
  const response = await fetch(path, {
    headers: { "Content-Type": "application/json" },
    ...options,
  });

  if (!response.ok) {
    const text = await response.text();
    throw new Error(text || response.statusText);
  }

  return await response.json();
}

function renderStatus(data) {
  const statusEl = document.getElementById("status");
  const badgeEl = document.getElementById("connectionState");

  if (statusEl) {
    statusEl.textContent = JSON.stringify(data, null, 2);
  }

  if (badgeEl) {
    badgeEl.textContent = data.mcu_connected ? "Connected" : "Disconnected";
    badgeEl.dataset.state = data.mcu_connected ? "ok" : "error";
  }
}

async function refreshStatus() {
  try {
    const data = await api("/api/status");
    renderStatus(data);
  } catch (err) {
    const statusEl = document.getElementById("status");
    const badgeEl = document.getElementById("connectionState");

    if (statusEl) {
      statusEl.textContent = err.toString();
    }

    if (badgeEl) {
      badgeEl.textContent = "Error";
      badgeEl.dataset.state = "error";
    }
  }
}

async function setLed(on) {
  await api("/api/led", {
    method: "POST",
    body: JSON.stringify({ on }),
  });
  await refreshStatus();
}

async function getState(side) {
  return await api(`/api/state/${side}`);
}

async function setState(side, state) {
  await api(`/api/state/${side}`, {
    method: "POST",
    body: JSON.stringify({ state }),
  });
  await refreshStatus();
}

async function getTemp(side) {
  return await api(`/api/temp/${side}`);
}

async function setMode(mode) {
  await api("/api/mode", {
    method: "POST",
    body: JSON.stringify({ mode }),
  });
  await refreshStatus();
}

async function stopAll() {
  await api("/api/stop", { method: "POST" });
  await refreshStatus();
}

function wireControls() {
  const ledSwitch = document.querySelector(".switch input[type='checkbox']");
  ledSwitch.addEventListener("change", () => setLed(ledSwitch.checked));

  document.querySelectorAll("[data-action='state']").forEach((armSwitch) => {
    armSwitch.addEventListener("change", () => setState(Number(armSwitch.dataset.side), armSwitch.checked));
  });

  document.querySelectorAll("[data-action='mode']").forEach((button) => {
    button.addEventListener("click", () => setMode(Number(button.dataset.mode)));
  });

  document.querySelectorAll("[data-action='stop']").forEach((button) => {
    button.addEventListener("click", stopAll);
  });
}

window.headsetApi = {
  getState,
  getTemp,
  setLed,
  setMode,
  setState,
  stopAll,
  refreshStatus,
};

wireControls();
setInterval(refreshStatus, 1000);
refreshStatus();
