// Tool dictionary
let tools = [
  { name: "claw hammer", image: "img/hammer1.jpg", type: "hammer", callname: "Tool_01_clawhammer"},
  { name: "rubber hammer", image: "img/hammer2.jpg", type: "hammer", callname: "Tool_02_rubberhammer"},
  { name: "hex screwdriver", image: "img/hexscrewdriver.png", type: "screwdriver", callname: "Tool_03_hexscrewdriver"},
  { name: "digital multimeter", image: "img/dmm.jpeg", type: "multimeter", callname: "Tool_05_digitalmultimeter"},
  { name: "clamp", image: "img/clamp.jpg", type: "clamp", callname: "Tool_06_clamp"},
  { name: "long nose pliers", image: "img/pliers.jpg", type: "pliers", callname: "Tool_07_longnosepliers"}
];

const middlemen = [
{ name: "Beacon2", ip: "http://172.20.10.2" },
{ name: "Beacon1", ip: "http://172.20.10.4" }
];

function showPopup(message, isLoading = false) {
closePopup();
let overlay = document.createElement("div");
overlay.className = "popup-overlay";
document.body.appendChild(overlay);

const popup = document.createElement("div");
popup.className = "popup";

popup.innerHTML = `
    <div class="popup-content">
        <p>${message}</p>
        ${isLoading ? '<div class="loader"></div>' : '<button onclick="closePopup()">OK</button>'}
    </div>
`;

document.body.appendChild(popup);
}

function closePopup() {
const popup = document.querySelector(".popup");
const overlay = document.querySelector(".popup-overlay");
if (popup) popup.remove();
if (overlay) overlay.remove();
}

function displayTools(filter = "") {
const toolContainer = document.querySelector(".tool-options");
if (!toolContainer) return;

toolContainer.innerHTML = "";

const filteredTools = tools.filter(tool =>
  tool.name.toLowerCase().includes(filter.toLowerCase())
);

if (filter.trim() === "") {
  toolContainer.innerHTML = "<p class='message'>🔍 Search for your lost tool!</p>";
  return;
}
else if (filteredTools.length === 0) {
  toolContainer.innerHTML = "<p class='message'>❌ No matching tools found.</p>";
  return;
}

filteredTools.forEach(tool => {
  const card = document.createElement("div");
  card.className = "tool-card";

  const img = document.createElement("img");
  img.src = tool.image;
  img.alt = tool.name;

  const name = document.createElement("span");
  name.textContent = tool.name;

  card.appendChild(img);
  card.appendChild(name);
  toolContainer.appendChild(card);

  card.addEventListener("click", () => {
    findmytool(tool);
  });
});
}

async function findmytool(tool) {
  console.log(`📨 Requesting tool: ${tool.callname}`);
  showPopup("🔍 Finding your tool...", true);

  try {
    const results = await Promise.allSettled(
      middlemen.map(async (m) => {
        try {
          const res = await fetch(`${m.ip}/find_tool`, {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ tool: tool.callname })
          });

          const data = await res.json();
          console.log(`✅ ${m.name} responded with RSSI`, data.rssi);
          return { status: "fulfilled", name: m.name, rssi: data.rssi };
        } catch (err) {
          console.warn(`⚠️ ${m.name} failed to respond`, err);
          return { status: "rejected", name: m.name, rssi: -999 };
        }
      })
    );

    // Normalize results
    const parsedResults = results.map(r =>
      r.status === "fulfilled"
        ? { name: r.value.name, rssi: r.value.rssi }
        : { name: r.reason?.name || "Unknown", rssi: -999 }
    );

    console.log("📊 All parsed results:", parsedResults);

    // Handle case where no middleman responded
    if (parsedResults.every(m => m.rssi === -999)) {
      document.querySelector(".popup-content").innerHTML = `
        <p>❌ No signal detected from any middleman.</p>
        <button onclick="closePopup()">OK</button>
      `;
      return;
    }

    // Safely find closest responder
    let closest = parsedResults[0];
    for (let i = 1; i < parsedResults.length; i++) {
      if (parsedResults[i].rssi > closest.rssi) {
        closest = parsedResults[i];
      }
    }

    // Build result display
    const popup = document.querySelector(".popup-content");
    if (!popup) {
      console.error("❌ Popup content container not found.");
      return;
    }

    // Display only the closest middleman
    popup.innerHTML = `
      <p>📡 <b>${tool.name}</b> is nearer to <b style="color: green">${closest.name}</b></p>
      <button onclick="closePopup()">OK</button>
    `;
  } catch (error) {
    console.error("🔥 Uncaught fetch error:", error);
    document.querySelector(".popup-content").innerHTML = `
      <p>⚠️ Failed to connect to the server.</p>
      <button onclick="closePopup()">OK</button>
    `;
  }
}

document.addEventListener("DOMContentLoaded", () => {
const searchInput = document.getElementById("search");
if (searchInput) {
  searchInput.addEventListener("input", (e) => {
    displayTools(e.target.value);
  });
}
displayTools("");
});