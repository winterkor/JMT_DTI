//declare some tools database
let toolsDB = [
    { name: "Hammer 1", type: "Hammer" },
    { name: "Hammer 2", type: "Hammer" },
    { name: "Screwdriver 1", type: "Screwdriver" },
    { name: "Screwdriver 2", type: "Screwdriver" },
    { name: "Digital Multimeter", type: "Multimeter" },
    { name: "Clamp", type: "Clamp" },
    { name: "Pliers", type: "Pliers" }
  ];
  
  function renderToolList() {
    const toolList = document.getElementById("toolList");
    toolList.innerHTML = "";
  
    toolsDB.forEach((tool, index) => {
      const card = document.createElement("div");
      card.className = "tool-card";
  
      card.innerHTML = `
        <div>
          <strong>${tool.name}</strong> (${tool.type})
        </div>
        <div class="actions">
          <button onclick="editTool(${index})">Edit</button>
          <button onclick="deleteTool(${index})">Delete</button>
        </div>
      `;
  
      toolList.appendChild(card);
    });
  }
  
  function saveTool() {
    const nameInput = document.getElementById("toolName");
    const typeInput = document.getElementById("toolType");
    const editIndex = document.getElementById("editIndex").value;
  
    const name = nameInput.value.trim();
    const type = typeInput.value.trim();
  
    if (name === "" || type === "") {
      alert("Please fill in both fields.");
      return;
    }
  
    if (editIndex === "") {
      // Create
      toolsDB.push({ name, type });
    } else {
      // Update
      toolsDB[editIndex] = { name, type };
      document.getElementById("editIndex").value = "";
    }
  
    nameInput.value = "";
    typeInput.value = "";
  
    renderToolList();
  }
  
  function editTool(index) {
    const tool = toolsDB[index];
    document.getElementById("toolName").value = tool.name;
    document.getElementById("toolType").value = tool.type;
    document.getElementById("editIndex").value = index;
  }
  
  function deleteTool(index) {
    if (confirm(`Are you sure you want to delete "${toolsDB[index].name}"?`)) {
      toolsDB.splice(index, 1);
      renderToolList();
    }
  }
  
  document.addEventListener("DOMContentLoaded", () => {
    renderToolList();
  });
  