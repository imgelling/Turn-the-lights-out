#include "game.h"

#define LIGHT_ON_COLOR game::Colors::White
#define LIGHT_OFF_COLOR game::Colors::DarkGray

class Game : public game::Engine
{

public:
	game::PixelMode pixelMode;  // Draws pixel by pixel
	uint32_t boardSize;			// Size of the board, 5 or 7 lights square
	game::Random random;		// Random number generator
	uint32_t seed;				// Seed for the game board, can be used to regen the same board
	bool *gameBoard;			// Holds data about the lights on the board, on or off
	uint32_t clicks;			// How many clicks the user has used to try and solve the board
	float_t time;				// Time it has taken the user to try and solve the board
	uint32_t attempts;			// Attempts the user has taken to try and solve the board
	bool hasWon;				// Has the user won?

	Game() : game::Engine()
	{
		boardSize = 9;
		gameBoard = nullptr;
		seed = 0;
		clicks = 0;
		time = 0;
		hasWon = false;
		attempts = 1;
	}

	void ResetBoard(const bool newSeed)
	{
		if (newSeed)
		{
			random.NewSeed();
			attempts = 1;
		}
		else
		{
			random.SetSeed(random.GetSeed());
			attempts++;
		}
		seed = random.GetSeed();
		clicks = 0;
		time = 0;
		hasWon = false;


		// Set a number of loops to do
		// Then instead of just making a light turn on
		// Act like the mouse is clicked and change
		// lights accordingly
		//for (uint32_t y = 0; y < boardSize; y++)
		//{
		//	for (uint32_t x = 0; x < boardSize; x++)
		//	{
		//		if (random.RndRange(0, 100) % 2)
		//		{
		//			gameBoard[y * boardSize + x] = true;
		//		}
		//		else
		//		{
		//			gameBoard[y * boardSize + x] = false;
		//		}
		//	}
		//}
		ZeroMemory(gameBoard, boardSize * boardSize);
		uint32_t x = 0;
		uint32_t y = 0;
		for (uint32_t loops = 0; loops < 5; loops++)
		{
			x = random.RndRange(0, boardSize - 1);
			y = random.RndRange(0, boardSize - 1);
			// Center
			DoLightUpdate(x, y);
			// Left
			DoLightUpdate(x - 1, y);
			// Up
			DoLightUpdate(x, y - 1);
			// Right
			DoLightUpdate(x + 1, y);
			// Down
			DoLightUpdate(x, y + 1);
		}
	}

	void DrawBoard()
	{
		// Draw board
		uint32_t scale = 360 / boardSize;
		game::Recti lightRect;
		uint32_t xOffset = pixelMode.GetPixelFrameBufferSize().width - 360;
		game::Color lightColor = game::Colors::DarkGray;
		for (uint32_t y = 0; y < boardSize; y++)
		{
			for (uint32_t x = 0; x < boardSize; x++)
			{
				lightRect.left = xOffset + x * scale;
				lightRect.top = y * scale;
				lightRect.right = lightRect.left + scale-1;
				lightRect.bottom = lightRect.top + scale-1;
				lightColor = gameBoard[y * boardSize + x] ? LIGHT_ON_COLOR : LIGHT_OFF_COLOR;
				pixelMode.RectFilledClip(lightRect, lightColor);
				pixelMode.RectClip(lightRect, game::Colors::Red);
			}
		}
	}

	bool CheckForWin() const
	{
		for (uint32_t light = 0; light < boardSize * boardSize; light++)
		{
			if (gameBoard[light]) return false;
		}
		return true;
	}

	bool DoLightUpdate(const uint32_t x, const uint32_t y)
	{
		if (x < 0) return false;
		if (x > boardSize - 1) return false;
		if (y < 0) return false;
		if (y > boardSize - 1) return false;
		gameBoard[y * boardSize + x] = !gameBoard[y * boardSize + x];
		return true;
	}

	void Initialize()
	{
		game::Attributes attributes;

		// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		//_CrtSetBreakAlloc(613);
#endif

		attributes.WindowTitle = "Turn the Lights Out";
		attributes.VsyncOn = true;

		geSetAttributes(attributes);
	}

	void LoadContent()
	{
		// Setup pixel mode
		if (!pixelMode.Initialize({ 640, 360 }))
		{
			geLogLastError();
		}

		gameBoard = new bool[9 * 9];
		//random.SetSeed((uint32_t)2504604244);
		ResetBoard(true);
	}

	void Shutdown()
	{
		delete[] gameBoard;
	}

	void Update(const float_t msElapsed)
	{
		game::Pointi scaledMousePos = pixelMode.GetScaledMousePosition();

		if (!hasWon)
		{
			time += msElapsed / 1000.0f;
		}

		// Handle Input
		if (geKeyboard.WasKeyReleased(geK_F11))
		{
			geToggleFullscreen();
		}
		if (geKeyboard.WasKeyReleased(geK_ESCAPE))
		{
			geStopEngine();
		}
		// Reset current seed/board
		if (geKeyboard.WasKeyReleased(geK_R))
		{
			ResetBoard(false);
		}
		// Get a new seed/board
		if (geKeyboard.WasKeyReleased(geK_N))
		{
			ResetBoard(true);
		}
		// Change size of board
		if (geKeyboard.WasKeyReleased(geK_S))
		{
			boardSize == 5 ? boardSize = 9 : boardSize = 5;
			ResetBoard(true);
		}

		// Press a light
		if (geMouse.WasButtonReleased(geMOUSE_LEFT) && !hasWon)
		{
			uint32_t posx = 0;
			uint32_t posy = 0;
			uint32_t scale = 360 / boardSize;
			posx = scaledMousePos.x - (pixelMode.GetPixelFrameBufferSize().width - 1 - 360);
			posx = posx / scale;
			posy = scaledMousePos.y / scale;

			if (posx < 0) return;
			if (posx > boardSize - 1) return;
			if (posy < 0) return;
			if (posy > boardSize - 1) return;

			// Center
			if (DoLightUpdate(posx, posy))
			{
				// DoLightUpdate will return true if the user
				// clicked on the game board, but only need
				// to check the original position
				clicks++;
			}
			// Left
			DoLightUpdate(posx - 1, posy);
			// Up
			DoLightUpdate(posx, posy - 1);
			// Right
			DoLightUpdate(posx + 1, posy);
			// Down
			DoLightUpdate(posx, posy + 1);

			hasWon = CheckForWin();
		}
	}

	void Render(const float_t msElapsed)
	{
		// Clears and starts new scene
		geClear(GAME_FRAME_BUFFER_BIT | GAME_DEPTH_STENCIL_BUFFER_BIT, game::Colors::Black);

		pixelMode.Clear(game::Colors::Blue);

		DrawBoard();

		pixelMode.TextClip("FPS: " + std::to_string(geGetFramesPerSecond()), 10, 10, game::Colors::White);
		pixelMode.TextClip("Seed: " + std::to_string(seed), 10, 20, game::Colors::White);
		pixelMode.TextClip("Clicks: " + std::to_string(clicks), 10, 30, game::Colors::White);
		pixelMode.TextClip("Time: " + std::to_string(time), 10, 40, game::Colors::White);
		pixelMode.TextClip("Attempts: " + std::to_string(attempts), 10, 50, game::Colors::White);

		if (hasWon)
		{
			pixelMode.Text("YOU WON!", 10, (360 >> 1) - (80 >> 1), game::Colors::Green, 10);
		}

		pixelMode.TextClip("Options:", 10, 290, game::Colors::White);
		pixelMode.TextClip("R - Reset current board.", 10, 300, game::Colors::White);
		pixelMode.TextClip("N - New board.", 10, 310, game::Colors::White);
		pixelMode.TextClip("S - Size of the board", 10, 320, game::Colors::White);
		pixelMode.TextClip("ESC - Quit", 10, 330, game::Colors::White);
		pixelMode.TextClip("F11 - Toggle full screen", 10, 340, game::Colors::White);


		pixelMode.Render();
	}
};

int main()
{
	game::Logger logger("Log.html");
	Game engine;

	engine.geSetLogger(&logger);

	// Create the needed bits for the engine
	if (!engine.geCreate())
	{
		engine.geLogLastError();
		return EXIT_FAILURE;
	}

	// Start the engine
	engine.geStartEngine();

	return EXIT_SUCCESS;
}