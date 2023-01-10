<script>
    let textVisible = false;
    let fileVissible = false;

    function textRadioClicked() {
        textVisible = true;
        fileVissible = false;
    }

    function fileRadioClicked() {
        fileVissible = true;
        textVisible = false;
    }

    let username = '';
    let keyword = '';
    let prefix = '';
    let data = '';
    let fileExtension ='';

    async function textInput(fileOrText) {
        const formData = new FormData();

        formData.append("input_1", username);
        if(fileOrText == 1) {
            formData.append("input_2", keyword);
            formData.append("input_3", prefix);
            formData.append("input_4", data);
        } else if(fileOrText == 2) {
            formData.append("input_5", fileExtension);
        }


        const response = await fetch('http://localhost:8000/', {
            method: 'POST',
            body: textInfo.get()
            });
        const result = await response.text();
        console.log(result);
    }

</script>
<body>
    <h1>
        <span class="twenty">21</span>
        <span class="e8">e8</span>
    </h1>
    <form>
        <span class="username"> Username: </span>
        <input type="text" class="username-input" bind:value={username}><br><br>
        <span class="file"> File </span>
        <input type="radio" class="file" on:click={fileRadioClicked} name="input-choice">
        <span class="text"> Text </span>
        <input type="radio" on:click={textRadioClicked}  name="input-choice"><br><br>
    </form>
    {#if textVisible}
        <div>
            <span class="keyword"> Key word: </span>
            <input type="text" bind:value={keyword}><br><br>
            <span class="prefix"> Prefix: </span><span>/////</span>
            <input type="text" bind:value={prefix}><br><br>
            <span class="data"> Data: </span><span>///////</span>
            <input type="text" bind:value={data}><br><br>
            <button on:click={textInput(1)}>Submit</button>
        </div>
    {/if}
    {#if fileVissible}
        <div>
            <span class="data"> Data(extension ie. '.png'): </span><span></span>
            <input type="text" bind:value={fileExtension}><br><br>
        </div>
        <button on:click={textInput(2)}>Submit</button>
    {/if}
</body>
<style>
body {
    text-align: center;
}

h1 {
    text-align: center;
}

h1
.twenty
{
    font-size: 70px;
    text-align: center;
    line-height: 200px;
    color: rgb(198, 203, 241);
    text-shadow:
        0 0 7px rgb(112, 123, 210),
        0 0 10px rgb(112, 123, 210),
        0 0 21px rgb(112, 123, 210),
        0 0 42px rgb(112, 123, 210),
        0 0 82px rgb(112, 123, 210),
        0 0 92px rgb(112, 123, 210),
        0 0 102px rgb(112, 123, 210),
        0 0 151px rgb(112, 123, 210);
}

h1
.e8
{
    font-size: 70px;
    text-align: center;
    line-height: 200px;
    color:  rgb(161, 221, 131);
    text-shadow:
        0 0 7px rgb(121, 211, 76),
        0 0 10px rgb(121, 211, 76),
        0 0 21px rgb(121, 211, 76),
        0 0 42px rgb(121, 211, 76),
        0 0 82px rgb(121, 211, 76),
        0 0 92px rgb(121, 211, 76),
        0 0 102px rgb(121, 211, 76),
        0 0 151px rgb(121, 211, 76);
}

input[type=text] {
    display: inline-block;
    border: 2px solid #dadada;
    border-radius: 7px;
}

input[type=text]:focus { 
    outline: none;
    border-color: #9ecaed;
    box-shadow: 0 0 10px #9ecaed;
}

.username{
    color: white;
}

.prefix {
    color: white;
}

.data {
    color: white;
}

.keyword {
    color: white;
}

.text{
    color: white;
}

.file{
    color: white;
}

form {
    display: inline-block;
}

:global(body) {
    background-color: black;
}
</style>