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
    let archiver_ready =  '';

    let reply;

    const postMine = async function mine (asdf) {
        let params = {}
        if (asdf === "asdf") {
            archiver_ready = "true"
        } else if(asdf === undefined){
            console.log("asdf: " + asdf)
        }else {
            archiver_ready = "false"
        }

        console.log("Archiver: " + archiver_ready)

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
        let json
        try {
            reply = await axios.post('http://localhost:5557' + "/mine", null, {params})

            let key_and_greatest;
            key_and_greatest = reply.data.split('/');
            let get_data = "";
            // await new Promise(r => setTimeout(r, 30000));
            get_data = await axios.get('http://localhost:5557' + "/send_data", {
                params: {
                    key_word_hash: key_and_greatest[0],
                    greatest: key_and_greatest[1]
                }
            })

            json =  JSON.stringify(get_data.data)

            } catch (e) {
                console.log(e);
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
            console.log([...json_map.entries()]);
            change_root(json_map)
    }

    function deleteFiles() {
        root[0].files.length = 0;
    }

    const addVendorTest = async (event) => {
        const formData = new FormData(event.target)
    
        // [Use formData]
        console.log([...formData]);

        event.target.reset();
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
                }
			]
		},
	];
    
    function addFile(){
        root[0].files.push({name: 'new file', files: [{name:'nothing'}]});
    }

    let data_hash
    function change_root(map) {
        let int_array
        let i = 0
        let file_contents
        console.log("Keys size: " + map.size)
        for (let [key, value] of map.entries()) {
            let seperate_bytes = value.split(",")
            int_array = seperate_bytes.map(x => parseInt(x));
            let bytes = new Buffer(int_array)
            file_contents = bytes.toString('hex')
            if(file_contents.length == 212) {
                prefix = key.substring(0,4)
                //checking for jpg, png, jpeg
                if(prefix == 'f814' || prefix == '8f8c' || prefix == '41e5') {
                    data_hash = file_contents.substring(132, 196)
                }
            }
            let base64String;
            for (let [key, value] of map.entries()) {
                console.log("Key: " + key)
                if(key === "file"){
                    console.log("entered")

                    let seperate_bytes = value.split(",")
                    int_array = seperate_bytes.map(x => parseInt(x))
                    let bytes = new Buffer(int_array)
                    base64String = bytes.toString('base64')
                    if(prefix == 'f814') {
                        document.getElementById("image").src = "data:image/jpg;base64," + base64String
                    } else if (prefix == '8f8c') {
                        document.getElementById("image").src = "data:image/png;base64," + base64String
                    } else if(prefix == '41e5') {
                        console.log("entered")
                        document.getElementById("image").src = "data:image/jpeg;base64," + base64String
                    }
                }
            }
            if(key === "file name") {
                root[0].name = value
            } else {
                addFile()
                root[0].files[i].name = key
                root[0].files[i].files[0].name = file_contents;
            }
            i++
        }

        return root
    }

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
        <div class="form">
            <form on:submit|preventDefault={addVendorTest} class="beginning-input">
                <span class="username"> Username: </span>
                <input type="text" class="username-input" bind:value={username}><br><br>
                <span class="file"> File </span>
                <input type="radio" class="file" on:click={fileRadioClicked} on:click={deleteFiles} name="input-choice">
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
                <br><br><button on:click={()=> postMine('asdf')}>Submit</button><br><br><br><br><br>
                <button on:click={deleteFiles}>Remove archive</button><br><br><br>
                <Folder name="Click for archived file" files={root} expanded/>
                <img id="image" src="" alt="Pic" width="300" height="450"/>
            </form>
        </div>
</body>
<style>

.form {
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
    text-align: center;
}

:global(body) {
    background-color: lightblue;
}
</style>