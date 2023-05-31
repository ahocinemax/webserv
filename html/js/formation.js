const prevButton =	document.querySelector(".prev-formation");
const nextButton =	document.querySelector(".next-formation");
const container =	document.querySelector(".image-main");
const formations =	Array.from(container.children);
formations.pop();
// console.log(document.querySelector(".sub-part h1").style);
const blocWidth =	container.clientWidth;

const setFormationPosition	= (formations, index) => 
{
	formations.style.left	= blocWidth * index + "px"
}
formations.forEach(setFormationPosition);

const moveToFormation		= (container, currentFormation, targetFormation) => 
{
	// console.log(container, currentFormation, targetFormation);
	formations[0].style.transform	= 'translateX(-' + targetFormation.style.left + ')';
	formations[1].style.transform	= 'translateX(-' + targetFormation.style.left + ')';
	formations[2].style.transform	= 'translateX(-' + targetFormation.style.left + ')';
	currentFormation.classList.remove("active");
	targetFormation.classList.add("active");
}

const updateArrow		= (container, prevButton, nextButton, targetIndex) =>
{
	// console.log(targetIndex);
	if (targetIndex === 0)
	{
		prevButton.classList.add("hidden");
	}
	else
	{
		prevButton.classList.remove("hidden");
	}
	if (targetIndex === formations.length - 1)
	{
		nextButton.classList.add("hidden");
	}
	else
	{
		nextButton.classList.remove("hidden");
	}
}

const moveRoutine 		= (container, formations, prevButton, nextButton, currentFormation, targetFormation, targetIndex) =>
{
	console.log(prevButton.classList);
	console.log(nextButton.classList);
	moveToFormation(container, currentFormation, targetFormation);
	updateArrow(container, prevButton, nextButton, targetIndex);
}

// When I click next button, move formations to the right
prevButton.addEventListener('click', e => {
	const currentFormation		= container.querySelector(".active");
	const prevFormation			= currentFormation.previousElementSibling;
	const prevIndex				= formations.findIndex(formation => formation === prevFormation);

	moveRoutine(container, formations, prevButton, nextButton, currentFormation, prevFormation, prevIndex)
});

// When I click prev button, move formations to the left
nextButton.addEventListener('click', e => {
	const currentFormation	= container.querySelector(".active");
	const nextformation			= currentFormation.nextElementSibling;
	const nextIndex			= formations.findIndex(formation => formation === nextformation);

	moveRoutine(container, formations, prevButton, nextButton, currentFormation, nextformation, nextIndex)
});