#include "Player.h"
#include "Engine.h"
#include "Textures.h"
#include "Input.h"
#include "Render.h"
#include "Log.h"
#include "Physics.h"
#include "Audio.h"
#include "Map.h"


Player::Player() : Entity(EntityType::PLAYER)
{
	name = "Player";
}

Player::~Player()
{}

bool Player::Awake()
{
	// L10: initial position stays in code (see the L10: TODO 4 config note) -- it's map data,
	// not player config, and L15 loads it from the map instead. Away from
	// the left wall (L09's Collisions layer).
	// L04: TODO 2: Initialize the player's position
	position = Vector2D(300.0f, 300.0f);
	return true;
}

bool Player::Start()
{
	// L10: TODO 5: Read texture, frame size and tuning from config.xml
	// instead of hardcoding them. GetSize() is not the fix for texW/texH --
	// the real texture (a spritesheet) would return the whole sheet's size,
	// not one frame's.
	texture = Engine::GetInstance().textures->Load(parameters.child("texture").attribute("path").as_string());
	texW = parameters.child("frame").attribute("width").as_int();
	texH = parameters.child("frame").attribute("height").as_int();

	// L10: TODO 5: Load this entity's animations from a TSX and set the current animation to "idle".
	anims.LoadFromTSX(parameters.child("animations").attribute("tsx").as_string(), { {0, "idle"}, {11, "move"}, {22, "jump"} });
	anims.SetCurrent("idle");

	// Initialize audio effect
	pickCoinFxId = Engine::GetInstance().audio->LoadFx(parameters.child("audiofx").attribute("path").as_string());

	// L08: TODO 5: Add physics to the player - create the physics body: a CAPSULE (CreateCapsule) with fixed rotation, so the player does not roll
	pbody = Engine::GetInstance().physics->CreateCircle((int)position.getX() + texW / 2, (int)position.getY() + texH / 2, texW / 2, bodyType::DYNAMIC);

	// L08: TODO 6: Assign player class (using "this") to the listener of the pbody. This makes the Physics module to call the OnCollision method
	pbody->listener = this;

	// L08: TODO 7: Assign collider type
	pbody->ctype = ColliderType::PLAYER;

	return true;
}

bool Player::Update(float dt)
{
	// L08: TODO 5: Add physics to the player - updated player position using physics

	if (!isDashing) {


		velocity = Engine::GetInstance().physics->GetLinearVelocity(pbody);
		velocity = { 0, velocity.y }; // Reset horizontal velocity by default, this way the player stops when no key is pressed

		// L10: TODO 7: Default to the "idle" animation; a key below overrides it
		anims.SetCurrent("idle");

		// Move left/right
		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT) {
			velocity.x = -speed;
			// L10: TODO 7: Switch to the "move" animation
			anims.SetCurrent("move");

		}
		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT) {
			velocity.x = speed;
			// L10: TODO 7: Switch to the "move" animation
			anims.SetCurrent("move");

		}

		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT && canJump == true) {
			velocity.y -= 10;
			anims.SetCurrent("move");//animation
			LOG("JUMPED");
			canJump = false;
			canDash = true;

		}


		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT) {
			velocity.y += 100;
			anims.SetCurrent("move");//animation
			Engine::GetInstance().physics->SetLinearVelocity(pbody, velocity);

		}

		Engine::GetInstance().physics->SetLinearVelocity(pbody, velocity);



	}


	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_REPEAT && canDash == true) {
			
			isDashing = true;
			Engine::GetInstance().physics->ApplyLinearImpulseToCenter(pbody, 5, 0);

			anims.SetCurrent("move");//animation
			LOG("DASHED");
			canDash = false;
			isDashing = false;
	}

	Engine::GetInstance().physics->SetLinearVelocity(pbody, velocity);

	// Apply velocity via helper

	int x, y;
	pbody->GetPosition(x, y);
	position.setX((float)x);
	position.setY((float)y);



	// L10: TODO 8: Camera follow
	Vector2D mapSize = Engine::GetInstance().map->GetMapSizeInPixels();
	float camW = Engine::GetInstance().render->camera.w;
	float camH = Engine::GetInstance().render->camera.h;
	float limitLeft = camW / 4.0f;
	float limitRight = mapSize.getX() - camW * 3.0f / 4.0f;
	float limitUp = camH / 4.0f;
	float limitDown = mapSize.getX() - camH * 3.0f / 4.0f;

	if (position.getX() > limitLeft && position.getX() < limitRight) {
		Engine::GetInstance().render->camera.x = camW / 4.0f - position.getX();
	}

	if (position.getY() > limitUp && position.getY() < limitDown) {
		Engine::GetInstance().render->camera.y = camH / 4.0f - position.getY();
	}

	// L10: TODO 6: Advance the animation clock -- logic, so it belongs in Update(), not Draw()
	anims.Update(dt);

	return true;
}

bool Player::Draw()
{
	// L08: TODO 5: Add physics to the player - updated player position using physics
	// L04: TODO 4: Render the player texture at its current position

	// L10: TODO 6: Draw the current animation frame instead of the whole spritesheet
	SDL_Rect currentFrame = anims.GetCurrentFrame();
	Engine::GetInstance().render->DrawTexture(texture, (int)position.getX() - texW / 2, (int)position.getY() - texH / 2, &currentFrame);

	return true;
}

bool Player::CleanUp()
{
	LOG("Cleanup player");
	// Release the physics body first, then the texture
	if (pbody != nullptr)
	{
		pbody->listener = nullptr;
		Engine::GetInstance().physics->DeletePhysBody(pbody);
		pbody = nullptr;
	}

	if (texture != nullptr)
	{
		Engine::GetInstance().textures->UnLoad(texture);
		texture = nullptr;
	}
	return true;
}

// L08: TODO 6: Define OnCollision function for the player.
void Player::OnCollision(PhysBody* physA, PhysBody* physB)
{
	// Handle collision with other physics bodies
	// For example, if the player collides with an item, you can implement logic here


	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:

		canJump = true;
		canDash = true;
		LOG("Collision PLATFORM");
		if (canJump == true)LOG("Can Jump");
		if (canDash == true)LOG("Can Dash");

		break;
	case ColliderType::ITEM:
		
		//Play the coin pickup sound effect 
		Engine::GetInstance().audio->PlayFx(pickCoinFxId);

		// Destroy the item. 
		physB->listener->Destroy(); 

		LOG("Collision ITEM");

		break;
	case ColliderType::UNKNOWN:
		LOG("Collision UNKNOWN");
		break;
	default:
		break;
	}

}
