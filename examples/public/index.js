const submitQuery = async () => {
   const res = await fetch("/func", { method: "POST" });
   const data = await res.text();
   document.getElementById("fetch-res-p").innerHTML = data;
}

document.getElementById("fetch-button").addEventListener("click", submitQuery);
