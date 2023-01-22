<script>
    import axios from 'axios';
    import Folder from './Folder.svelte';
    import {Buffer} from 'buffer';

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
        let json;
        try{
            reply = await axios.post('http://localhost:5557' + "/mine", null, {params})

            // console.log("data: " + reply.data)
            let get_data = "";
            while(get_data == "") {
                get_data = await axios.get('http://localhost:5557' + "/send_data", {
                    params: {
                        key_word_hash: reply.data
                    }
                })
                json = JSON.stringify(get_data.data)
            }
            } catch (e) {
                error = e;
            }
            // console.log(json)
            let json_length = Object.keys(json).length
            json = json.replace(/"/g, '')
            json = json.replace(/}/g, '')
            json = json.substring(1, json_length)
            json = json.split("]")
            const json_map = new Map()
            for(let i = 0; i < json.length - 1; i++) {
                let json_pairs = json[i].split(":")
                json_pairs[0] = json_pairs[0].replace(",", '')
                json_pairs[1] = json_pairs[1].replace("[", '')
                json_map.set(json_pairs[0], json_pairs[1])
            }
            change_root(json_map)
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
			name: 'Nothing to display',
			files: [
				{ 
                    name: 'nothing to display',
                    files: [
                        {name: 'nothing'}
                    ]    
                },
                { 
                    name: 'nothing to display',
                    files: [
                        {name: 'nothing'}
                    ]     
                },
				{ 
                    name: 'nothing to display',
                    files: [
                        {name: 'nothing'}
                    ]    
                },
                { 
                    name: 'nothing to display',
                    files: [
                        {name: 'nothing'}
                    ]    
                },
				{ 
                    name: 'nothing to display',
                    files: [
                        {name: 'nothing'}
                    ]    
                }
			]
		},
	];

    let data_hash
    function change_root(map) {
        let int_array
        let i = 0
        for (let [key, value] of map.entries()) {
            let seperate_bytes = value.split(",")
            int_array = seperate_bytes.map(x => parseInt(x));
            // const ints = new Array(int_array);
            let bytes = new Buffer(int_array)
            let file_contents
            // console.log(hex)
            file_contents = bytes.toString('hex')
            // console.log("Key: " + key)
            
            if(file_contents.length == 212) {
                prefix = key.substring(0,4)
                //checking for jpg, png, jpeg
                if(prefix == 'f814' || prefix == '8f8c' || prefix == '41e5') {
                    data_hash = file_contents.substring(132, 196)
                }
            }
            for (let [key, value] of map.entries()) {
                // console.log(key === data_hash)
                // console.log("Data hash: " + data_hash)
                if(key === data_hash){
                    let seperate_bytes = value.split(",")
                    int_array = seperate_bytes.map(x => parseInt(x));
                    let bytear_array = new Uint8Array(int_array)
                    console.log("Arraylength: "+int_array.length)
                    console.log(bytear_array)
                    let bytes = new Buffer(bytear_array)
                    let base64String = bytes.toString('base64');
                    document.getElementById("image").src = "data:image/png;base64," + base64String;
                }
            }
            if(key == "file name") {
                root[0].name = value
            } else {
                
                root[0].files[i].name = key
                root[0].files[i].files[0].name = file_contents;
            }
            
            i++
        }

        return root
    }

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
	import { exclude_internal_props, select_option } from 'svelte/internal';
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
    <div class="send-input">
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
            <span class="prefix"> Prefix: </span>
            <input type="text" bind:value={prefix} id="inputprefix"><br><br>
            <span class="data"> Data: </span>
            <input type="text" bind:value={data} id="inputdata"><br><br>
            <button on:click={postMine}>More input</button>
        </div>
    {/if}
    {#if fileVissible}
        <div>
            <span class="keyword"> Key word: </span>
            <input type="text" bind:value={keyword} id="inputkeyword2"><br><br>
            <span class="data"> File name: </span>
            <input type="text" bind:value={fileExtension} id="inputFileextension"><br><br>
            <button on:click={postMine}>More input</button>
        </div>
    {/if}
        <br><br><button on:click={make_archiver_ready} on:click={postMine} on:click{change_root}>Submit</button><br><br><br>
    </form>
    </div>

    
</body>
<div class="explorer" svelte:style="z-index: 1;">
    <Folder name="Click for archived file" files={root} expanded/>
    <img id="image" src="" alt="Pic" width="300" height="450"/>
</div>
<style>
body {
    text-align: center;
    position: absolute;
    top: 50%;
    left: 50%;
    transform: translate(-50%, -50%);
}

.send-input{
    margin-right: 500px;
    margin-top: -100px;
}

.explorer {
    padding-left: 900px;
    margin-top: -200px;
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

form {
    display: inline-block;
}

:global(body) {
    background-color: lightblue;
}
</style>