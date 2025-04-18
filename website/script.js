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
{ name: "M1", ip: "http://172.20.10.2" },
{ name: "M2", ip: "http://172.20.10.4" }
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
  console.log(`Requesting tool: ${tool.callname}`);
  showPopup("🔍 Finding your tool...", true);

  try {
    const results = await Promise.allSettled(
      middlemen.map(async (m) => {
        try {
          const res = await fetch(`${m.ip}/ping_tool`, {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ tool: tool.callname })
          });

          const data = await res.json();
          return { status: "fulfilled", name: m.name, rssi: data.rssi };
        } catch (err) {
          return { status: "rejected", name: m.name, rssi: -999 };
        }
      })
    );

    // Filter and normalize
    const parsedResults = results.map(r =>
      r.status === "fulfilled"
        ? { name: r.name, rssi: r.rssi }
        : { name: r.name, rssi: -999 }
    );

    // Find closest
    const closest = parsedResults.reduce((a, b) =>
      b.rssi > a.rssi ? b : a
    );

    // Build result display
    const allRSSIList = parsedResults
      .map(m => `${m.name}: ${m.rssi === -999 ? '❌ No signal' : m.rssi}`)
      .join("<br>");

    document.querySelector(".popup-content").innerHTML = `
      <p>📡 First to get signal: <b style="color: green">${closest.name}</b></p>
      <p><b>RSSI Comparison:</b><br>${allRSSIList}</p>
      <button onclick="closePopup()">OK</button>
    `;
  } catch (error) {
    console.error("Fetch error:", error);
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
