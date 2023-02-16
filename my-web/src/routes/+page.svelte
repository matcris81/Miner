<script setContext=module>
    import radial_data from './data.js'
    import { postMine } from './module.svelte';
    import { prefix as prefixes } from './variableModule.svelte';
    
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

    function text_or_file(decision) {
        if(decision == true) {
            textVisible = true
        } else if (decision == false) {
            textVisible = false
        }
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

    let files;
    $: if (files) {
    console.log(files);
        for (const file of files) {
            console.log(`${file.name}: ${file.size} bytes`);
        }
    }

    let index = 0
    const updateData = () => {
        if(index == radial_data.length) {
            index = 0
        }
        radial_data[index].name = keyword;
        index++
    };

    let username = ''
    let keyword = ''

    let fileExtension = ''
    function handleFileRadioChange(event) {
        radioValue = event.target.value;
        keyword = '';
        fileExtension = ''
    }

    let prefix = ''
    let data = ''
    function handleTextRadioChange(event) {
        radioValue = event.target.value;
        keyword = '';
        prefix = ''
        data = ''
    }

    function addPrefix() {
        if(prefixes.length >= 5) {
            
        }
        prefixes.push(prefix)
    }

</script>
<body>
    <div class="form">
        <form on:submit|preventDefault={addVendorTest} class="beginning-input">
            <span class="username"> Username: </span>
            <input type="text" class="username-input" bind:value={username}><br><br>
            <span class="file"> File </span>
            <input type="radio" class="file" on:click={fileRadioClicked} on:click={() => text_or_file(false)} on:change={handleTextRadioChange} name="input-choice">
            <span class="text"> Text </span>
            <input type="radio" on:click={textRadioClicked} on:click={() => text_or_file(true)} on:change={handleFileRadioChange} name="input-choice"><br><br>
        {#if textVisible}
            <div>
                <span class="keyword"> Key word: </span>
                <input type="text" bind:value={keyword} id="inputkeyword" /><br><br>
                <span class="prefix"> Prefix: </span>
                <input type="text" bind:value={prefix} id="inputprefix" /><br><br>
                <span class="data"> Data: </span>
                <input type="text" bind:value={data} id="inputdata" /><br><br>
                <button on:click={() => postMine(username, keyword, data, fileExtension, 'as' , textVisible)}>More input</button>
            </div>
        {/if}
        {#if fileVissible}
            <div>
                <span class="keyword"> Key word: </span>
                <input type="text" bind:value={keyword} id="inputkeyword2" /><br><br>
                <span class="data"> File name: </span>
                <input type="text" bind:value={fileExtension} id="inputFileextension" /><br><br>
                <button on:click={() => postMine(username, keyword, data, fileExtension, 'as' , textVisible)} >More input</button>
            </div>
        {/if}
            <br><br><button on:click|preventDefault={() => postMine(username, keyword, data, fileExtension, 'asdf' , textVisible)} on:click={updateData}>Submit</button><br><br>
            <!-- <video on:load={() => objectURL = URL.createObjectURL(new Blob([new Uint8Array(seperate_bytes)], { type: 'video/mp4' }))}>
                <source src={objectURL} type="video/mp4" />
            </video> -->
        </form>
    </div>
</body>
<style>

.form {
    text-align: center;
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