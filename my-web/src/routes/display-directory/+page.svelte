<script>
	import options from '../data.js';
	import Selector from '../selector.svelte';
	import Folder from '../Folder.svelte'
	import { root } from '../module.svelte';
	let selected = 0;


	function deleteFiles() {
        root[0].files.length = 0;
    }
	let showFolder = false
	function changeFolder() {
		showFolder = true
	}

	let picTrue = true

</script>

<style>
* {
  padding:0;
margin: 0;
}
body {
  text-align: center;
  font-family: "Roboto";
  color: #fff;
  letter-spacing:1px;
}

.container {
  display: flex;
  justify-content: center;
  align-items: center;
  height: 10vh;
}

a {
  font-size: 1.5rem;
  padding: 1rem 3rem;
  color: #f4f4f4;
  text-transform: uppercase;
}

.btn {
  text-decoration: none;
  border: 1px solid rgb(146, 148, 248);
  position: relative;
  overflow: hidden;
}

.btn:hover {
  box-shadow: 1px 1px 25px 10px rgba(146, 148, 248, 0.4);
}

.btn:before {
  content: "";
  position: absolute;
  top: 0;
  left: -100%;
  width: 100%;
  height: 100%;
  background: linear-gradient(
    120deg,
    transparent,
    rgba(146, 148, 248, 0.4),
    transparent
  );
  transition: all 650ms;
}

.btn:hover:before {
  left: 100%;
}


/* div 
.btn-1{
	top:20px;
	height: 100px;
	width: 500px;
}


.archive{
	height: 100px;
	width: 500px;
	border: 1px solid white;
	padding: 30px;
	margin: auto;
	
} */

	
</style>
<body>
	<div style="display: flex; justify-content: center;align-items: center; height:30vh; position:relative; margin-top:20px">
		<Selector options={options} bind:selected on:change={changeFolder}/>
	</div>
	<div style="display: flex; justify-content: center; align-items: center; height: 0vh; padding-left: 0%">
		<!-- <button on:click={deleteFiles}>Remove archive</button> -->
	</div>
	<div class="container">
		<a href="#" class="btn">
		<Folder name="Click for archived file" files={root} expanded/>
		</a>
	</div>

	<br><br><br><br><br><br><br><br><br><br>
	{#if picTrue}
	<div style="display: flex; justify-content: center; align-items: center; height:30vh; padding-left: 20%">
		<img id="image" src="" alt="Pic" width="300" height="450"/>
	</div>
	{/if}
</body>