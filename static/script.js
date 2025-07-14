// scripts.js

function showTagSuggestions() {
    const input = document.getElementById("searchInput").value.toLowerCase();
    const suggestionsDiv = document.getElementById("tagSuggestions");
    suggestionsDiv.innerHTML = "";

    if (input === "") return;

    const matchingTags = tags.filter(tag => tag.toLowerCase().includes(input));
    matchingTags.forEach(tag => {
        const span = document.createElement("span");
        span.textContent = "#" + tag;
        span.style = "display:inline-block; background:#eef; color:#333; padding:4px 8px; margin:4px; border-radius:10px; cursor:pointer;";
        span.onclick = () => {
            document.getElementById("searchInput").value = tag;
            suggestionsDiv.innerHTML = "";
        };
        suggestionsDiv.appendChild(span);
    });
}
