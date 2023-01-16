<script>
    import axios from 'axios';
    import Folder from './Folder.svelte';

    let textVisible = false;
    let fileVissible = false;

    function textRadioClicked(event) {
        textVisible = true;
        fileVissible = false;
        event.stopPropagation();
    }

    function fileRadioClicked(event) {
        fileVissible = true;
        textVisible = false;
        event.stopPropagation();
    }

    function clearTextInputs() {
        document.getElementById("inputkeyword").value = "";
        document.getElementById("inputprefix").value = "";
        document.getElementById("inputdata").value = "";
    }

    function clearFileInputs() {
        document.getElementById("inputkeyword2").value = "";
        document.getElementById("inputFileextension").value = "";
    }

    let username = '';
    let keyword = '';
    let prefix = '';
    let data = '';
    let fileExtension ='';
    let archiver_ready =  'false';

    const postMine = async() =>{
        let params = {}

        if(textVisible == true) {
            params = {
                input_1: username,
                input_2: keyword,
                input_3: prefix,
                input_4: data,
                input_5: archiver_ready,
            };
        } else if(fileVissible == true) {
            params = {
                input_1: username,
                input_2: keyword,
                input_3: fileExtension,
                input_4: archiver_ready,
            };
        }

        const reply = axios.post('http://localhost:5557' + "/mine", null, {params}).then((res)=>{res}).catch((e)=>{console.error(e)})
    }
    const addVendorTest = async (event) => {
        const formData = new FormData(event.target)
    
        // [Use formData]
        console.log([...formData]);

        event.target.reset();
    }

    function make_archiver_ready() {
        archiver_ready = "true";
    }

    let root = [
		{
			name: 'Important work stuff',
			files: [
				{ name: 'quarterly-results.xlsx' }
			]
		},
	];

    // const express = require('express');
    // const app = express();
    // const bodyParser = require('body-parser');
    // const fs = require('fs');

</script>
<body>
    <h1>
        <span class="twenty">21</span>
        <span class="e8">e8</span>
    </h1>
    <form on:submit|preventDefault={addVendorTest}>
        <span class="username"> Username: </span>
        <input type="text" class="username-input" bind:value={username}><br><br>
        <span class="file"> File </span>
        <input type="radio" class="file" on:click={fileRadioClicked} name="input-choice">
        <span class="text"> Text </span>
        <input type="radio" on:click={textRadioClicked}  name="input-choice"><br><br>
    <!-- </form> -->
    {#if textVisible}
    <!-- <form> -->
        <div>
            <span class="keyword"> Key word: </span>
            <input type="text" bind:value={keyword} id="inputkeyword"><br><br>
            <span class="prefix"> Prefix: </span><span>/////</span>
            <input type="text" bind:value={prefix} id="inputprefix"><br><br>
            <span class="data"> Data: </span><span>///////</span>
            <input type="text" bind:value={data} id="inputdata"><br><br>
            <button on:click={postMine}>More input</button>
        </div>
    {/if}
    {#if fileVissible}
        <div>
            <span class="keyword"> Key word: </span>
            <input type="text" bind:value={keyword} id="inputkeyword2"><br><br>
            <span class="data"> Data(extension ie. '.png'): </span><span></span>
            <input type="text" bind:value={fileExtension} id="inputFileextension"><br><br>
            <button on:click={postMine}>More input</button>
        </div>
    {/if}
        <br><br><button on:click={make_archiver_ready} on:click={postMine}>Submit</button><br><br><br>
    </form>
    <div class="fileExplorer">
        <Folder name="Home" files={root} expanded/>
    </div>
</body>
<style>
body {
    text-align: center;
    position: absolute;
    top: 50%;
    left: 50%;
    transform: translate(-50%, -50%);
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

.fileExplorer {
    display: flex;
    height: 300px;
    width: 500px;
    justify-content: center;
    align-items: center;
    background-color: white;
}

:global(body) {
    background-color: lightblue;
}
</style>