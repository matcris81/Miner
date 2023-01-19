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

    let reply;

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
        try{
            reply = await axios.post('http://localhost:5557' + "/mine", null, {params})

            // console.log("data: " + reply.data)
                let get_data = "";
                let json;
                while(get_data == "") {
                    get_data = axios.get('http://localhost:5557' + "/send_data", {
                        params: {
                            key_word_hash: reply.data
                        }
                    })
                    json = JSON.stringify(get_data)
                    console.log("Json: " + json)
                }
        } catch (e) {
            error = e;
        }

        console.log("Data:" + reply.data);

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
    
    $: if (files) {
        // Note that `files` is of type `FileList`, not an Array:
        // https://developer.mozilla.org/en-US/docs/Web/API/FileList
        console.log(files);

        for (const file of files) {
            console.log(`${file.name}: ${file.size} bytes`);
        }
    }

    let root = [
		{
			name: 'Important work stuff',
			files: [
				{ name: 'quarterly-results.xlsx' }
			]
		},
		{
			name: 'Animal GIFs',
			files: [
				{
					name: 'Dogs',
					files: [
						{ name: 'treadmill.gif' },
						{ name: 'rope-jumping.gif' }
					]
				},
				{
					name: 'Goats',
					files: [
						{ name: 'parkour.gif' },
						{ name: 'rampage.gif' }
					]
				},
				{ name: 'cat-roomba.gif' },
				{ name: 'duck-shuffle.gif' },
				{ name: 'monkey-on-a-pig.gif' }
			]
		},
		{ name: 'TODO.md' }
	];

    let selectedFiles;
    function handleFileSelection() {
        let file = selectedFiles[0];
        let reader = new FileReader();
        reader.onload = (e) => {
            let contents = e.target.result;
            console.log(contents);
        };
        reader.readAsText(file);
    }

    // import x from '../../../mining/x.jpg';
    // let img = ;
    import { onMount } from "svelte"
	import { select_option } from 'svelte/internal';
      // d3.csv(' http://127.0.0.1:8081/test.csv').then(function(data) {
      //   console.log(data[0])})
    
      let files;
      $: if (files) {
        console.log(files);
        for (const file of files) {
          console.log(`${file.name}: ${file.size} bytes`);
        }
      }

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
    {#if textVisible}
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
            <span class="data"> Data(extension ie. '.png'): </span>
            <input type="text" bind:value={fileExtension} id="inputFileextension"><br><br>
            <button on:click={postMine}>More input</button>
        </div>
    {/if}
        <br><br><button on:click={make_archiver_ready} on:click={postMine}>Submit</button><br><br><br>
    </form>

    <Folder name="Home" files={root} expanded/>

    <input type='file' bind:files>
    
    {#if files}
        <h2>Selected files:</h2>
        {#each Array.from(files) as file, i}
            <p>{file.name} {file.size} bytes</p>
                {#await file.text() then text}
                    <p>Inside text: {text}</p>
                {/await}
            {/each}
        <p>files length: {files.length}</p>
    {/if}

    <!-- {#each Array.from(files) as file, i}
  <p>{file.name} {file.size} bytes</p>
  {#await file.text() then text}
    <p>e: {text} i: {i}</p>
  {/await}
{/each} -->

    <!-- <input bind:files id="many" multiple type="file" class="files"/>

    {#if files}
        <h2 class="selected">Selected files:</h2>
        {#each Array.from(files) as file}
            <p class="file-name">{file.name} ({file.size} bytes) </p>
        {/each}
    {/if} -->

    <br><br>

    <!-- <img src={x} alt="chosenImage" class="image"> -->
    
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
    text-align: center;
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

/* .selected{
    color: white;
}

.file-name{
    color: white;
}

.files{
    color: white;
} */

form {
    display: inline-block;
}

/* .image{
    width: 200px;
    height: 200px;
    border-radius:50%;
} */

:global(body) {
    background-color: lightblue;
}
</style>