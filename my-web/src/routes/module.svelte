<script context="module">
    import {Buffer} from 'buffer'
    import axios from "axios"
    import { spv_lists } from "./variableModule.svelte"
    import { prefix as prefixes } from './variableModule.svelte'

    export let root = [
		{
			name: 'Nothing to display',
			files: [
				{ 
                    name: 'nothing to display',
                    files: [
                        {name: 'nothing'},
                    ]
                },
                {
                    name: 'nothing to display',
                    files: [
                        { name: 'quarterly-results.xlsx' }
                    ]
                },
                {
                    name: 'nothing to display',
                    files: [
                        { name: 'quarterly-results.xlsx' }
                    ]
                },
                {
                    name: 'nothing to display',
                    files: [
                        { name: 'quarterly-results.xlsx' }
                    ]
                },
                {
                    name: 'nothing to display',
                    files: [
                        { name: 'quarterly-results.xlsx' }
                    ]
                }

			]
		},
	];

    export async function postMine(username, keyword, data, fileExtension, asdf, textVisible) {
        let params = {}
		let archiver_ready = ''
        if (asdf === "asdf") {
            archiver_ready = "true" 
        } else if(asdf === undefined){
            console.log("asdf: " + asdf)
        } else {
            archiver_ready = "false"
        }

        if(textVisible == true) {
            params = {
                input_1: username,
                input_2: keyword,
                input_3: prefixes,
                input_4: data,
                input_5: archiver_ready,
            };
        } else if(textVisible == false) {
            params = {
                input_1: username,
                input_2: keyword,
                input_3: fileExtension,
                input_4: archiver_ready,
            };
        }
        let key_greatest_prefix = []
        let reply
        try {
            reply = await axios.post('http://localhost:5557' + "/mine", null, {params})
            
            key_greatest_prefix = reply.data.split(/[/ ]/);
            prefixes.push(key_greatest_prefix[2])
            console.log("prefix 1: "+prefixes)
            let get_data = ""
            if(asdf == 'asdf') {
                get_data = await axios.get('http://localhost:5557' + "/spv_list", {
                    params: {
                        key_word_hash: key_greatest_prefix[0],
                        greatest: key_greatest_prefix[1]
                    }
                })
                let spv_list
                spv_list =  JSON.stringify(get_data.data)
                spv_list = spv_list.replace(/"/g, '')
                spv_lists.push(spv_list)
                console.log(spv_lists)
            }
            
            let j = 0
            
        } catch (e) {
            console.log(e);
        }
    }

	export async function directory(name, index) {
        let current_spv_list = []

        for(let i = 0; i < spv_lists[index].length; i+=64) {
            current_spv_list.push(spv_lists[index].substr(i ,64))
        }
		let params = {}
		params = {
				directory_name: name
            };
		let reply
        let bytes
        let all_bytes = []
        let bytes_array = []
        let directory_contents
		try {
			reply = await axios.post('http://localhost:5557' + "/extract_directory", null, {params})
            for(let i = 0; i < current_spv_list.length; i++) {
                bytes = await axios.get('http://localhost:5557' + "/get_file_bytes", {
                    params: {
                        key_word_hash: reply.data,
                        file: current_spv_list[i]
                    }
                })
                all_bytes.push(JSON.stringify(bytes.data))
            }
            for(let i = 0; i < all_bytes.length; i++) {
                all_bytes[i] = all_bytes[i].replace(/]/g, '')
                all_bytes[i] = all_bytes[i].replaceAll('[', '')
                bytes_array += all_bytes[i]
            }
            console.log("prefix 2: " + prefixes)
            displayPicture(bytes_array, prefixes[index])
            directory_contents = await axios.get('http://localhost:5557' + "/directory_contents", {
                params: {
                    directory_name: reply.data
                }  
            })
            load_directory(reply.data, directory_contents.data)

		} catch(e) {
			console.log(e)
		}
	}

    function displayPicture(all_bytes, prefix) {
        let int_array
        let seperate_bytes = all_bytes.split(",")
        int_array = seperate_bytes.map(x => parseInt(x))
        let bytes = new Buffer(int_array)
        let base64String = bytes.toString('base64')
        if(prefix == 'f814') {
            document.getElementById("image").src = "data:image/jpg;base64," + base64String
        } else if (prefix == '8f8c') {
            document.getElementById("image").src = "data:image/png;base64," + base64String
        } else if(prefix == '41e5') {
            document.getElementById("image").src = "data:image/jpeg;base64," + base64String
        }
    }

    function addFile(){
        root[0].files.push({name: 'new file', files: [{name:'nothing'}]});
    }

    function load_directory(directory_name, files) {
        console.log(directory_name)
        console.log(files)
        root[0].name = directory_name
        for(let i = 0; i < files.length; i++) {
            if(i > 5) {
                addFile()
            }
            root[0].files[i].name = files[i][0]
            root[0].files[i].files[0].name = files[i][1].toString('hex');
        }

        return root
    }
</script>