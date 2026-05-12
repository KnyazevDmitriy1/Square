#include <iostream>
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>

//константы с размером экрана
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCORE_ZONE = 30;
//очки
int SCORE = 0;

class Text
{
public:
	//Инициализация переменных
	Text();

	//Очистка памяти
	~Text();

	//Загрузка и отрисовка текста
	bool loadText(int currentScore);

	//создание картинки из string
	bool loadFromRenderedText(std::string textureText, SDL_Color textColor);

	//Удаление текстуры
	void free();

	void render();

private:
	//текстура
	SDL_Texture* mTexture;

	//Размеры
	int mWidth;
	int mHeight;
};

class Square
{
public:
	//Размеры квадрата
	static const int SQUARE_SIZE = SCREEN_WIDTH / 10;

	//Скорость квадрата
	static const int SQUARE_VEL = 10;

	//Инициализация переменных
	Square();

	//Обработка нажатия клавиш
	void handleEvent(SDL_Event& e);

	//Двигает квадрат
	void move(SDL_Rect& green_square);

	//Отрисовка квадрата
	void render();

	//Получение collision box
	SDL_Rect getCollider() const; //const гарантирует, что не изменяет поля класса

private:

	//Координаты квадрата
	int mPosX, mPosY;

	//Скорость квадрата
	int mVelX, mVelY;

	//Collision box чёрного квадрата
	SDL_Rect mCollider;
};

bool init();

void close();

bool checkCollision(SDL_Rect a, SDL_Rect b);

void greenSquare(SDL_Rect& green_square, const Square& square);

void greenSquareRender(SDL_Rect& green_square);

//окно
SDL_Window* window = NULL;

SDL_Surface* screenSurface = NULL;

SDL_Renderer* renderer = NULL;

TTF_Font* font = NULL;

Text textTexture;

Square::Square()
{
	//Координаты
	mPosX = (SCREEN_WIDTH - SQUARE_SIZE) / 2;
	mPosY = (SCREEN_HEIGHT - SQUARE_SIZE) / 2;

	//Размер collision box
	mCollider.w = SQUARE_SIZE;
	mCollider.h = SQUARE_SIZE;

	//Скорость
	mVelX = 0;
	mVelY = 0;
}

void Square::handleEvent(SDL_Event& e)
{
	//Если клавиша нажата
	if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
	{
		//Поднять скорость
		switch (e.key.keysym.sym)
		{
		case SDLK_UP: mVelY -= SQUARE_VEL; break;
		case SDLK_DOWN: mVelY += SQUARE_VEL; break;
		case SDLK_LEFT: mVelX -= SQUARE_VEL; break;
		case SDLK_RIGHT: mVelX += SQUARE_VEL; break;
		}
	}
	//Если клавиша отпущена
	else if (e.type == SDL_KEYUP && e.key.repeat == 0)
	{
		//Убрать скорость
		switch (e.key.keysym.sym)
		{
		case SDLK_UP: mVelY += SQUARE_VEL; break;
		case SDLK_DOWN: mVelY -= SQUARE_VEL; break;
		case SDLK_LEFT: mVelX += SQUARE_VEL; break;
		case SDLK_RIGHT: mVelX -= SQUARE_VEL; break;
		}
	}
}

void Square::move(SDL_Rect& green_square)
{
	//Движение квадрата влево или вправо
	mPosX += mVelX;
	mCollider.x = mPosX;

	//Если квадрат ушёл слышком далеко влево или вправо или задел зелёный квадрат
	if ((mPosX < 0) || (mPosX + SQUARE_SIZE > SCREEN_WIDTH) || checkCollision(mCollider,green_square))
	{
		//если задел зелёный квадрат
		if (checkCollision(mCollider, green_square))
		{
			SCORE++;
			textTexture.loadText(SCORE);
			greenSquare(green_square, *this); //this - объект для которого вызывается функция в которой этот this написан
		}
		//если задел стену
		else
		{
			//Движение назад
			mPosX -= mVelX;
			mCollider.x = mPosX;
		}
	}

	//Движение квадрата вверх или вниз
	mPosY += mVelY;
	mCollider.y = mPosY;

	//Если квадрат ушёл слышком далеко вверх или вниз или задел зелёный квадрат
	if ((mPosY < SCORE_ZONE) || (mPosY + SQUARE_SIZE > SCREEN_HEIGHT) || checkCollision(mCollider,green_square))
	{
		//если задел зелёный квадрат
		if (checkCollision(mCollider, green_square))
		{
			SCORE++;
			textTexture.loadText(SCORE);
			greenSquare(green_square, *this);
		}
		//если задел стену
		else
		{
			//Движение назад
			mPosY -= mVelY;
			mCollider.y = mPosY;
		}
	}
}

void Square::render()
{
	//отрисовка чёрного квадрата
	SDL_Rect square = { mPosX, mPosY, SQUARE_SIZE, SQUARE_SIZE };
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 1);
	SDL_RenderFillRect(renderer, &square);
}

SDL_Rect Square::getCollider() const //const гарантирует, что не изменяет поля класса
{
	return mCollider;
}

Text::Text()
{
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

Text::~Text()
{
	free();
}

bool Text::loadFromRenderedText(std::string textureText, SDL_Color textColor)
{
	//очистить если существует
	free();

	SDL_Surface* textSurface = TTF_RenderText_Solid(font, textureText.c_str(), textColor);
	if (textSurface == NULL)
	{
		std::cout << "Не удалось отрисовать text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
	}
	else
	{
		mTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
		if (mTexture == NULL)
		{
			std::cout << "Не удалось создать текстуру из отрисованного текста! SDL Error: " << SDL_GetError() << std::endl;
		}
		else
		{
			//Получить размеры
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}

		//Удаление старого surface
		SDL_FreeSurface(textSurface);
	}

	return mTexture != NULL;
}

void Text::free()
{
	if (mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

bool Text::loadText(int currentScore)
{
	bool success = true;

	if (font == NULL)
	{
		std::cout << "Шрифт не загружен!" << std::endl;
		success = false;
	}
	else
	{
		SDL_Color textColor = { 0, 0, 0 };
		std::string scoreText = "Score: " + std::to_string(currentScore);
		if (!loadFromRenderedText(scoreText, textColor))
		{
			std::cout << "Не удалось отрисовать текстуру текста" << std::endl;
			success = false;
		}
	}

	return success;
}

void Text::render()
{
	SDL_Rect renderRect = { 0, 0, mWidth, mHeight };

	SDL_RenderCopy(renderer, mTexture, NULL, &renderRect);
}

//запуск SDL и создание окна
bool init()
{
	bool success = true;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cout << "SDL не может инициализироваться! SDL_Error: " << SDL_GetError() << std::endl;
		success = false;
	}
	else
	{
		//Создание окна
		window = SDL_CreateWindow("square", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (window == NULL)
		{
			std::cout << "Не получилось создать окно! SDL_Error: " << SDL_GetError() << std::endl;
			success = false;
		}
		else
		{
			//создание рендерера для окна
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (renderer == NULL)
			{
				std::cout << "Не удалось создать рендерер! SDL_Error: " << SDL_GetError() << std::endl;
				success = false;
			}
			else
			{
				SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

				//Инициализация TTF
				if (TTF_Init() == -1)
				{
					std::cout << "Не удалось инициализировать SDL_ttf! SDL_ttf Error: " << TTF_GetError() << std::endl;
					success = false;
				}

				//Загрузка шрифта
				font = TTF_OpenFont("arial.ttf", 30);
				if (font == NULL)
				{
					std::cout << "Не удалось загрузить шрифт! SDL_ttf Error:" << TTF_GetError() << std::endl;
					success = false;
				}
			}
			//получить поверхность окна
			screenSurface = SDL_GetWindowSurface(window);
			//заполнить поверхность(холст) белым
			SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
			//обновить поверхность(холст)
			SDL_UpdateWindowSurface(window);
		}
	}
	return success;
};

void close()
{
	textTexture.free();

	TTF_CloseFont(font);
	font = NULL;

	//уничтожить рендерер и окно
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	window = NULL;
	renderer = NULL;

	//выйти из SDL
	TTF_Quit();
	SDL_Quit();
}

bool checkCollision(SDL_Rect a, SDL_Rect b)
{
	//Стороны прямоугольников
	int leftA, leftB;
	int rightA, rightB;
	int topA, topB;
	int bottomA, bottomB;

	//Вычисление координат сторон прямоугольника A
	leftA = a.x;
	rightA = a.x + a.w;
	topA = a.y;
	bottomA = a.y + a.h;

	//Вычисление координат сторон прямоугольника B
	leftB = b.x;
	rightB = b.x + b.w;
	topB = b.y;
	bottomB = b.y + b.h;

	//Если хотя бы одна сторона A выходит за пределы B
	if (bottomA <= topB)
	{
		return false;
	}

	if (topA >= bottomB)
	{
		return false;
	}

	if (rightA <= leftB)
	{
		return false;
	}

	if (leftA >= rightB)
	{
		return false;
	}

	//Если ни одна сторона A не выходит за пределы B
	return true;
}

void greenSquare(SDL_Rect& green_square, const Square& square)
{
	//Размер зелёного квадрата
	int green_size = (SCREEN_WIDTH / 10) / 2;

	bool goodPosition = false;

	while (!goodPosition)
	{
		//Координаты зелёного квадрата
		int green_x = rand() % ((SCREEN_WIDTH - green_size) - 10 + 1) + 10;
		int green_y = rand() % ((SCREEN_HEIGHT - green_size) - SCORE_ZONE + 1) + SCORE_ZONE;

		//Зелёный квадрат
		green_square = { green_x,green_y,green_size,green_size };

		if (!checkCollision(green_square, square.getCollider()))
		{
			goodPosition = true;
		}
	}
}

void greenSquareRender(SDL_Rect& green_square)
{
	//установка цвета
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	//Очистка экрана установленным цветом
	SDL_RenderClear(renderer);

	//отрисовка зелёного квадрата
	SDL_SetRenderDrawColor(renderer, 0, 200, 0, 1);
	SDL_RenderFillRect(renderer, &green_square);
}

int main(int argc, char* argv[])
{
	if (!init())
	{
		std::cout << "Не удалось инициализировать!" << std::endl;
	}
	else
	{
		if (!textTexture.loadText(SCORE))
		{
			std::cout << "не удалось загрузить текст!" << std::endl;
		}
		else
		{
			srand(time(0));
			//Флаг главного цикла
			bool quit = false;

			//Обработчик событий
			SDL_Event e;

			//Чёрный квадрат
			Square square;

			//Зелёный квадрат
			SDL_Rect green_square;

			greenSquare(green_square, square);

			while (!quit)
			{
				while (SDL_PollEvent(&e) != 0)
				{
					if (e.type == SDL_QUIT)
					{
						quit = true;
					}
					square.handleEvent(e);
				}

				square.move(green_square);

				greenSquareRender(green_square);

				square.render();

				textTexture.render();

				//обновление экрана
				SDL_RenderPresent(renderer);
			}
		}
	}

	//освободить ресурсы и закрыть SDL
	close();

	return 0;
}