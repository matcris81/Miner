<script>
	import { directory } from "./module.svelte";

	export let options = [];
	export let selected;
	
	$: length = options.length;
	
	const cos = (i) => Math.sin(((Math.PI * 2) / length) * i);
	const sin = (i) => Math.cos(((Math.PI * 2) / length) * i);

</script>

<style>
	fieldset {
		border: 0;
		display: block;
		float: left;
		height: 100%;
		position: relative;
		shape-outside: circle(50%);
		width: 200px;
	}	
	label {
		display: inline-block;
		height: 100px;
		left: calc((50% - 50px) + (80px * var(--sin)));
		position: absolute;
		top: calc((50% - 50px) + (80px * var(--cos)));
		transition: top 500ms, left 500ms;
		width: 100px;
	}	
	
	label:hover > input + div,
	label:focus-within > input + div {
		border-color: #0000FF;
	}

	fieldset:not(:hover) > label {
		--cos: 0 !important;
		--sin: 0 !important;
	}

	fieldset:not(:hover) > label:not(.selected) {
		z-index: -1;
	}
	
	input {
		clip: rect(1px, 1px, 1px, 1px);  	
		position: absolute;
  	overflow: hidden;
	}

	input + div {
		border: 1px solid gray;
		border-radius: 50%;
		cursor: pointer;
		display: flex;
		background: #FFF;
		height: 100px;
		color: #000;
		overflow: hidden;
		position: relative;
		width: 100px;
	}

	input + div > span {
		display: block;
		font-size: 1.2rem;
		font-weight: bold;
		letter-spacing: 1px;
		position: absolute;
		text-align: center;
		top: 50%;
		left: 50%;
		transform: translate(-50%, -50%);
	}
</style>

<fieldset>
	{#each options as {img, name}, i}	
		<label class:selected={selected === i} 
			style="--cos:{cos(i)}; --sin:{sin(i)};"
			tabindex="-1"
			on:click={(event) => {
				event.preventDefault();
				directory(name, i)
			}}
			on:keydown={(event) => {
				event.preventDefault();
				directory(name, i)
			}}
			>
			<input type="radio" bind:group={selected} value={i}>
			<div>				
				<span>{name}</span>
			</div>
		</label>
	{/each}
</fieldset>
