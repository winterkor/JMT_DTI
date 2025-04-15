// Tool dictionary
let tools = [
    { name: "claw hammer", image: "img/hammer1.jpg", type: "hammer", callname: "Tool_01_clawhammer"},
    { name: "rubber hammer", image: "img/hammer2.jpg", type: "hammer", callname: "Tool_02_rubberhammer"},
    { name: "hex screwdriver", image: "img/hexscrewdriver.png", type: "screwdriver", callname: "Tool_03_hexscrewdriver"},
    // { name: "flathead screwdriver", image: "flatheadscrewdriver.jpg", type: "screwdriver", callname: "Tool_04_flatheadscrewdriver"},
    { name: "digital multimeter", image: "img/dmm.jpeg", type: "multimeter", callname: "Tool_05_digitalmultimeter"},
    { name: "clamp", image: "img/clamp.jpg", type: "clamp", callname: "Tool_06_clamp"},
    { name: "long nose pliers", image: "img/pliers.jpg", type: "pliers", callname: "Tool_07_longnosepliers"}
  ];

// Function to show the popup
function showPopup(message, isLoading = false) {
  closePopup();
  let overlay = document.createElement("div");
  overlay.className = "popup-overlay";
  document.body.appendChild(overlay);

  // document.body.style.pointerEvents = "none";  

  // Create popup container
  const popup = document.createElement("div");
  popup.className = "popup";

  // Set the popup content
  popup.innerHTML = `
      <div class="popup-content">
          <p>${message}</p>
          ${isLoading ? '<div class="loader"></div>' : '<button onclick="closePopup()">OK</button>'}
      </div>
  `;

  document.body.appendChild(popup);
}

// Function to close the popup and re-enable interaction
function closePopup() {
  const popup = document.querySelector(".popup");
  const overlay = document.querySelector(".popup-overlay");
    
  if (popup) popup.remove();
  if (overlay) overlay.remove();

  // document.body.style.pointerEvents = "auto";
}

 // Display tools in index.html
function displayTools(filter = "") {
    const toolContainer = document.querySelector(".tool-options");
    if (!toolContainer) return; // prevent errors on tools.html
  
    toolContainer.innerHTML = ""; // Clear previous
  
    const filteredTools = tools.filter(tool =>
      tool.name.toLowerCase().includes(filter.toLowerCase())
    );
  
    if (filter.trim() === "") {
      // display 'empty panel and ask user to search for their lost tool'
      toolContainer.innerHTML = "<p class='message'>🔍 Search for your lost tool!</p>";
      return; // Don't display anything if search empty or no match
    }
    else if (filteredTools.length === 0) {
      // display 'searched tool is not in list of available tagged tools 
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

      card.addEventListener("click", ()=> {
        findmytool(tool);
      })
    });
  }
  
  // Function to find and buzz a tool
async function findmytool(tool) {
  console.log(`Requesting tool: ${tool.callname}`);
  // Show loading popup
  showPopup("🔍 Finding your tool...", true);
  try {
      let response = await fetch("http://172.20.10.2/find_tool", { // Flask server URL
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify({ tool: tool.callname }),
      });
      // const result = await response.json();
      // Update the popup with the success or failure message
      if (response.ok) {
          document.querySelector(".popup-content").innerHTML = `
              <p>✅ ${tool.name} buzzed successfully!</p> 
              <h4>Detected nearest Bluetooth beacon <b style="color: red;">3</b></h4>
              <button onclick="closePopup()">OK</button>
          `;
      } else {
          document.querySelector(".popup-content").innerHTML = `
              <p>❌ Failed to buzz ${tool.name}.</p> 
              <button onclick="closePopup()">OK</button>
          `;
      }
  } catch (error) {
      console.error("Fetch error:", error);
      document.querySelector(".popup-content").innerHTML = `
          <p>⚠️ Failed to connect to the server.</p>
          <button onclick="closePopup()">OK</button>
      `;
  }
}
  
  // Search event listener for index.html
  document.addEventListener("DOMContentLoaded", () => {
    const searchInput = document.getElementById("search");
    if (searchInput) {
      searchInput.addEventListener("input", (e) => {
        displayTools(e.target.value);
      });
    }
    // Call displayTools with an empty string to show the default message on page load
    displayTools("");
  });