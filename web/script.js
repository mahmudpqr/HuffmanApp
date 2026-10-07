const inputText = document.getElementById("inputText");
const encodeButton = document.getElementById("encodeButton");

const codesContainer =
    document.getElementById("codesContainer");

const encodedBits =
    document.getElementById("encodedBits");

const decodeButton =
    document.getElementById("decodeButton");

const decodedText =
    document.getElementById("decodedText");

const status =
    document.getElementById("status");


function showStatus(message) {
    status.textContent = message;
}


function createForm(valueName, value) {

    const params = new URLSearchParams();

    params.append(valueName, value);

    return params;
}


encodeButton.addEventListener("click", async () => {

    const text = inputText.value;

    if (text.length === 0) {

        showStatus("Please enter some text.");

        return;
    }


    encodeButton.disabled = true;

    showStatus("Encoding...");


    try {

        const response = await fetch(
            "/api/encode",
            {
                method: "POST",

                headers: {
                    "Content-Type":
                        "application/x-www-form-urlencoded"
                },

                body: createForm(
                    "text",
                    text
                )
            }
        );


        const data =
            await response.json();


        if (!response.ok || !data.success) {

            throw new Error(
                data.error ||
                "Encoding failed."
            );
        }


        let html = `
            <table class="code-table">
                <thead>
                    <tr>
                        <th>Character</th>
                        <th>Code</th>
                    </tr>
                </thead>
                <tbody>
        `;


        for (const item of data.codes) {

            html += `
                <tr>
                    <td>${escapeHtml(item.character)}</td>
                    <td>${escapeHtml(item.code)}</td>
                </tr>
            `;
        }


        html += `
                </tbody>
            </table>
        `;


        codesContainer.innerHTML = html;


        encodedBits.value =
            data.encoded_bits;


        decodedText.value = "";


        showStatus("Encoding completed.");

    }
    catch (error) {

        showStatus(
            "Error: " + error.message
        );
    }
    finally {

        encodeButton.disabled = false;
    }
});


decodeButton.addEventListener("click", async () => {

    const bits =
        encodedBits.value;


    if (bits.length === 0) {

        showStatus(
            "Please enter an encoded bit string."
        );

        return;
    }


    if (!/^[01]+$/.test(bits)) {

        showStatus(
            "The encoded string must contain only 0 and 1."
        );

        return;
    }


    decodeButton.disabled = true;

    showStatus("Decoding...");


    try {

        const response = await fetch(
            "/api/decode",
            {
                method: "POST",

                headers: {
                    "Content-Type":
                        "application/x-www-form-urlencoded"
                },

                body: createForm(
                    "bits",
                    bits
                )
            }
        );


        const data =
            await response.json();


        if (!response.ok || !data.success) {

            throw new Error(
                data.error ||
                "Decoding failed."
            );
        }


        decodedText.value =
            data.decoded_text;


        showStatus("Decoding completed.");

    }
    catch (error) {

        showStatus(
            "Error: " + error.message
        );
    }
    finally {

        decodeButton.disabled = false;
    }
});


function escapeHtml(value) {

    return value
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;")
        .replace(/"/g, "&quot;")
        .replace(/'/g, "&#039;");
}