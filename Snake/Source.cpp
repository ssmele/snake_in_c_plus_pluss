#include <iostream>
#include <vector>
#include <SFML\Graphics.hpp>

using namespace std;
using namespace sf;

// Setting up game constants
constexpr unsigned int window_width(820), window_height(820);
constexpr float treat_width(20.0), treat_height(20);
constexpr float snake_body_width(20.0), snake_body_height(20.0);
constexpr auto window_title = "Snake!";

// TODO: Need to fine a good way to mangae fonts.
constexpr auto font_location = "C:/Users/stone/Documents/Libs/SFML-2.5.1/examples/shader/resources/sansation.ttf";

struct Rectangle {
	/*
	Struc that represents the base shape within my snake game.
	*/
	RectangleShape shape;
	float x() { return shape.getPosition().x; }
	float y() { return shape.getPosition().y; }

	float left() { return x() - shape.getSize().x / 2.f; }
	float right() { return x() + shape.getSize().x / 2.f; }
	float top() { return y() - shape.getSize().y / 2.f; }
	float bottom() { return y() + shape.getSize().y / 2.f; }
};

Vector2f get_random_position() {
	/* Method to get a random position on the game board. */
	auto max = window_height;
	auto min = 0;
	float x = (max - min) * ((((float)rand()) / (float)RAND_MAX)) + min;

	max = window_width;
	float y = (max - min) * ((((float)rand()) / (float)RAND_MAX)) + min;

	return Vector2f(x, y);
}

struct Treat : public Rectangle {
	/*
	Strcut that represents a treat within in the game.
	*/

	Treat(float x, float y) {
		shape.setPosition({ x, y });
		shape.setSize({ treat_width, treat_height });
		shape.setFillColor(Color::Magenta);
		shape.setOrigin(treat_width / 2, treat_height / 2);
	}


	void update() {
		shape.setPosition(get_random_position());
	}
};

struct SnakeBody : public Rectangle {
	/*
	Struct that represents a single portion of the snake body.
	*/

	SnakeBody(float x, float y) {
		shape.setPosition({ x, y });
		shape.setSize({ snake_body_width, snake_body_height });
		shape.setFillColor(Color::Green);
		shape.setOrigin(snake_body_width / 2, snake_body_height / 2);
	}
};

enum Direction { up, down, left, right };

struct Snake {
	/*
	Struct that represents the snake within the snake game.
	*/

	vector<SnakeBody> body;
	Direction dir = Direction::right;
	bool grow_next_update = false;

	Snake() {
		/* Method to initalize the snake in the middle of the screen. */
		body.push_back(SnakeBody(window_width / 2, window_height / 2));
	}

	SnakeBody get_head() {
		/*
		Method to get the head of the snake.
		*/
		return body.front();
	}

	bool self_collision() {
		/*
		Method to detect if self collision has occured.
		*/
		if (body.size() <= 4) return false;

		// Using cool c++ lambda method to see if head is in same position as any of the body.
		auto head = body.front();
		return body.end() != find_if(next(body.begin(), 4), body.end(), [=](auto portion){return has_collided(head, portion);});
	}

	void update() {
		// Gather the head of the snake.
		auto& head = body.front();
		
		// Figure out which direction we need to move in.
		if (Keyboard::isKeyPressed(Keyboard::Key::Up) && dir != Direction::down && dir != Direction::up) dir = Direction::up;
		else if (Keyboard::isKeyPressed(Keyboard::Key::Down) && dir != Direction::down && dir != Direction::up)  dir = Direction::down;
		else if (Keyboard::isKeyPressed(Keyboard::Key::Left) && dir != Direction::right && dir != Direction::left)  dir = Direction::left;
		else if (Keyboard::isKeyPressed(Keyboard::Key::Right) && dir != Direction::right && dir != Direction::left) dir = Direction::right;

		// Go through and update the snakes positions.
		SnakeBody prev = head;
		for (int i = 1; i < body.size(); i++) {
			auto next_prev = body[i];
			body[i].shape.setPosition({ prev.x(), prev.y() });
			prev = next_prev;
		}

		// Flag to let the snake know that it needs to grow the next update.
		if (grow_next_update) {
			body.push_back(SnakeBody(prev.x(), prev.y()));
			grow_next_update = false;
		}

		// Move in updated direction.
		switch (dir) {
			case Direction::up: 
				head.shape.setPosition({ head.x(), head.y() - snake_body_height });
				break;
			case Direction::down: 
				head.shape.setPosition({ head.x(), head.y() + snake_body_height});
				break;
			case Direction::right:
				head.shape.setPosition({ head.x() + snake_body_height, head.y() });
				break;
			case Direction::left:
				head.shape.setPosition({ head.x() - snake_body_height, head.y() });
				break;
		}
	}
};

bool has_collided(Rectangle r1, Rectangle r2) {
	/*
	Method that checks for collision between two Rectangles
	*/
	bool lhs_collision = r1.right() > r2.left() && r1.left() < r2.left();
	bool rhs_collision = r1.left() < r2.right() && r1.right() > r2.left();
	bool top_side_collision = r1.top() < r2.bottom() && r1.bottom() > r2.top();
	bool bottom_side_collision = r1.bottom() > r2.top() && r1.bottom() < r2.top();

	return (lhs_collision && (top_side_collision || bottom_side_collision)) || (rhs_collision && (top_side_collision || bottom_side_collision));
}


enum GameStates {start, playing, lost, exiting};

void load_game_font(Font * font) {
	if (!font->loadFromFile(font_location))
	{
		cout << "Error loading font" << endl;
		exit(1);
	}
}

void setup_text(Text * text, const String& string, Font * f, int char_size, Color fill_color, bool stylize) {
	text->setString(string);
	text->setFont(*f); // font is a sf::Font
	text->setCharacterSize(char_size); // in pixels, not points!
	text->setFillColor(fill_color);
	if (stylize) {
		text->setStyle(sf::Text::Bold | sf::Text::Underlined);
	}
}

GameStates start_screen(RenderWindow* window) {
	window->setFramerateLimit(15);

	// Generate start text and place in center.
	Text text;
	Font f;
	load_game_font(&f);
	setup_text(&text, "Press Enter to Play.", &f, 24, Color::Red, true);
	text.setPosition(window_width / 2, window_height / 2);
	FloatRect text_bounds = text.getLocalBounds();
	text.setOrigin(text_bounds.left + text_bounds.width / 2.0f, text_bounds.top + text_bounds.height / 2.0f);

	// Start the start screen loop.
	while (true) {
		window->clear(Color::Black);

		// See if the user is attempting to exit the game.
		if (Keyboard::isKeyPressed(Keyboard::Key::Escape))
			return GameStates::exiting;
		
		// See if the user is attempting to exit the game.
		if (Keyboard::isKeyPressed(Keyboard::Key::Enter))
			return GameStates::playing;

		window->draw(text);
		// Display all updates to the screen.
		window->display();
	}
}

GameStates start_game(RenderWindow* window, int * cur_high_score) {
	window->setFramerateLimit(30);

	//Initalize the snake.
	Snake snake;

	// Set up the first treat.
	auto rando_position = get_random_position();
	Treat treat(rando_position.x, rando_position.y);

	// Generate start text and place in center.
	Text text;
	Font f;
	load_game_font(&f);
	setup_text(&text, "", &f, 24, Color::Red, true);
	char score_buffer[31];
	int score = 0;

	// Game loop.
	while (true) {
		window->clear(Color::Black);

		// See if the user is attempting to exit the game.
		if (Keyboard::isKeyPressed(Keyboard::Key::Escape))
			return GameStates::exiting;

		// If we are out of bounds exit.
		SnakeBody snake_head = snake.get_head();
		if (snake_head.right() > window_width || snake_head.left() < 0 || snake_head.top() < 0 || snake_head.bottom() > window_height) {
			if (score > *cur_high_score) *cur_high_score = score;
			return GameStates::lost;
		}

		// Update the treat and snake.
		snake.update();

		// If the snake head has collided with the 
		if (has_collided(treat, snake.body.front())) {
			treat.update();
			snake.grow_next_update = true;
			score++;
		}

		if (snake.self_collision()) {
			if (score > *cur_high_score) *cur_high_score = score;
			return GameStates::lost;
		}


		// Draw all portions of the snake
		for (auto portion : snake.body) window->draw(portion.shape);

		// Draw the treat.
		window->draw(treat.shape);

		sprintf_s(score_buffer, sizeof(score_buffer), "Score: %d", score);
		text.setString(score_buffer);
		window->draw(text);

		// Display all updates to the screen.
		window->display();
	}


	return GameStates::start;
}

GameStates lost_game(RenderWindow* window, int high_score) {

	window->setFramerateLimit(15);

	// Display to the user that they lost.
	Text you_lost_text;
	Font f;
	load_game_font(&f);
	setup_text(&you_lost_text, "You Lost. Press Enter to try again.", &f, 24, Color::Red, true);
	you_lost_text.setPosition(window_width / 2, window_height / 2);
	FloatRect text_bounds = you_lost_text.getLocalBounds();
	you_lost_text.setOrigin(text_bounds.left + text_bounds.width / 2.0f, text_bounds.top + text_bounds.height / 2.0f);

	Text high_score_text;
	setup_text(&high_score_text, "Current High Score:" + to_string(high_score), &f, 24, Color::Yellow, true);
	high_score_text.setPosition(window_width / 2, (window_height / 2) + 2*(text_bounds.height));
	text_bounds = high_score_text.getLocalBounds();
	high_score_text.setOrigin(text_bounds.left + text_bounds.width / 2.0f, text_bounds.top + text_bounds.height / 2.0f);

	// Start the start screen loop.
	while (true) {
		window->clear(Color::Black);

		// See if the user is attempting to exit the game.
		if (Keyboard::isKeyPressed(Keyboard::Key::Escape))
			return GameStates::exiting;

		// See if the user is requesting to try again.
		if (Keyboard::isKeyPressed(Keyboard::Key::Enter))
			return GameStates::playing;

		window->draw(you_lost_text);
		window->draw(high_score_text);
		window->display();
	}
}


int main() {


	// Set up the game window.
	RenderWindow window{ {window_width, window_height}, window_title };

	int high_score = 0;

	GameStates cur_game_state{ start };
	while (true) {
		switch (cur_game_state) {
			case GameStates::start : 
				cur_game_state = start_screen(&window);
				break;
			case GameStates::playing :
				cur_game_state = start_game(&window, &high_score);
				break;
			case GameStates::lost :
				cur_game_state = lost_game(&window, high_score);
				break;
			case GameStates::exiting :
				return 0;
		}
	}
}