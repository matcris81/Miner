<script>
	import options from '../data.js';
	import Selector from '../selector.svelte';
	import Folder from '../Folder.svelte'
	import { root } from '../module.svelte';
	let selected = 0;


	function deleteFiles() {
        root[0].files.length = 0;
    }

</script>

<style>
	div {
		height: 250px;
	}
</style>
<body>
	<div style="display: flex; justify-content: center;align-items: center; height:30vh; position:relative;">	
		<Selector options={options} bind:selected/>
		<Folder name="Click for archived file" files={root} expanded/>
	</div>
	<div style="display: flex; justify-content: center; align-items: center; height: 0vh; padding-left: 0%">
		<button on:click={deleteFiles}>Remove archive</button>
	</div>
	<br><br><br><br><br><br><br><br><br><br>
	<div style="display: flex; justify-content: center; align-items: center; height:30vh; padding-left: 20%">
		<img id="image" src="" alt="Pic" width="300" height="450"/>
	</div>
</body>