# MazeGame
C++ based project, simple 3D game. Vulkan API example code used to draw image.

Use "make" to compile the programm

Use "make compile_shaders" to compile shaders

Use "make all" to compile all

Use "make clean" to clean compiled binary files

# Primary TODO list for the project:

	TODO: make UI  (in process)
		description:
			Windows and buttons, that display in-game info
			and collect user input

			1) Make a wrapping above the ImGui (almost done)
			2) Make a class that will manage the preparation of UI (not started)

			3) Fix some bugs with windows appering due to screen size change


	TODO: make MECH_MANGER (not started yet)
		decription:
			class or bunch of classes, that organize game proccess
			like level preparation, object spawning and destruction (though objs are owned by gameField, 
			but not managed by it)

			1) Make sure we know what we want be in the game, how it will be look like (in proccess)
			2) Create MECH_MANAGER, imply all our ideas about game process (not started yet)


	TODO: Texture and model loading arrangement (in proccess)
		description:
			As it is poorly arranged on current stage of the project
			it will be hard to expand the list of 3D models in the next stages
			So it definitely needs to be organized in the way to avoid much 
			hardcoded chuncks (for example it could be managed to load models from files) 
			
			1) Arrange satisfactory system of texture and model loading(in process)
			2) Make Models for the game (not started yet)


**EXTRA TODO LIST***

	TODO: Animation implementation

	TODO: Sound effects implementation




