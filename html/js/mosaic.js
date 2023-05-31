const mosaic				= document.querySelector(".main-container");
const projectImages			= mosaic.children;

const markup				= document.querySelector(".main-project");
const projects				= markup.children;

mosaic.addEventListener('click', e => 
{
	// Test if I clicked on a target
	const targetProject		= e.target.closest('div').classList[0];
	if (!targetProject) return ;

	// test if I already clicked on a project. If so, make it hide
	const displayedProject	= document.querySelector(".active");
	if (displayedProject)
	{
		displayedProject.classList.add("hide");
		displayedProject.classList.remove("active");
	}

	// display the chosen project, by removing hide class
	const projectToDisplay	= document.querySelector('.' + targetProject + '.main-project');
	projectToDisplay.classList.remove("hide");
	projectToDisplay.classList.add("active");

    // scroll to the displayed project
    const sizeToScroll      = mosaic.clientHeight + 75;
    window.scrollTo(0, sizeToScroll);
});
