const submitQuery = () => {
   const nameInput = document.getElementById("name-input");
   const passwordInput = document.getElementById("password-input");
   if (nameInput.value != "" && passwordInput != "") {
      fetch("/", { method: "POST" });
   }
}

document.getElementById("fetch-button").addEventListener("click", submitQuery);
