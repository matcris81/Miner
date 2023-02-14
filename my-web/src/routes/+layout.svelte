<body>
	<h1>
		<span class="twenty">21</span>
		<span class="e8">e8</span>
	</h1>
    <div id="navContainer">
        <a href="/">Home</a>
        <a href="/display-directory">Directory</a>
    </div>
	<slot />
</body>

<style>
	h1,
	.twenty {
		font-size: 70px;
		text-align: center;
		line-height: 200px;
		color: rgb(198, 203, 241);
		text-shadow: 0 0 7px rgb(112, 123, 210), 0 0 10px rgb(112, 123, 210),
			0 0 21px rgb(112, 123, 210), 0 0 42px rgb(112, 123, 210), 0 0 82px rgb(112, 123, 210),
			0 0 92px rgb(112, 123, 210), 0 0 102px rgb(112, 123, 210), 0 0 151px rgb(112, 123, 210);
	}

	h1,
	.e8 {
		font-size: 70px;
		text-align: center;
		line-height: 200px;
		color: rgb(161, 221, 131);
		text-shadow: 0 0 7px rgb(121, 211, 76), 0 0 10px rgb(121, 211, 76), 0 0 21px rgb(121, 211, 76),
			0 0 42px rgb(121, 211, 76), 0 0 82px rgb(121, 211, 76), 0 0 92px rgb(121, 211, 76),
			0 0 102px rgb(121, 211, 76), 0 0 151px rgb(121, 211, 76);
	}
	h1 {
		margin: 0;
	}
	body {
		background-color: lightblue;
	}
	#navContainer {
		display: flex;
		justify-content: center;
		gap: 2rem;
		font-size: 2rem;
		margin-bottom: 2rem;
	}
</style>
