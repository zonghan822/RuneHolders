#pragma once

// Engine Component(s)
#include "../editor/eno_scene.hpp"
#include "../editor/eno_scene_tree.hpp"

#include "../eno_core_host/eno_core_host.hpp"

#include "../scripting/eno_script_manager.hpp"

// Events
#include "../editor/editor_events.hpp"


namespace Editor
{
	class EnoEditor
	{
	public:

		EnoEditor() = default;
		EnoEditor(const EnoEditor&) = delete;
		EnoEditor(EnoEditor&&) = delete;

		EnoEditor(Event::EnoEventBus& bus, ECS::EntityManager& entMng, Core::EnoCoreHost& coreHost)
			:
			bus_ { bus },
			entMng_{ entMng },
			coreHost_{ coreHost },
			playingScene_{ false }
		{
			// Creates 1 scene by default if it is not loaded save data
			createScene();

			// First scene will be selected first
			selectedScene_ = scenes_.front().get();
		}

		// Testing the scene tree
		void operation()
		{
			//Node* node1 = tree_.create();
			//Node* node2 = tree_.create();
			//tree_.select(0);

			//tree_.nest(node2);

			//Node* node3 = tree_.create();
			//Node* node4 = tree_.create();
			//Node* node5 = tree_.create();
			//Node* node6 = tree_.create();
			//Node* node7 = tree_.create();

			//tree_.nest(node4, node1);
			//tree_.nest(node6, node2);

			//tree_.detach(node6, node3);
			//tree_.nest(node5, node6);

			//tree_.kill(node1);

			//// prints nothing, selected node was destroyed, which is
			//// expected behaviour
			//tree_.selectedNodeView();

			//// Create and open a new script
			////scriptMng_.createAndOpenScript("EnemyController");

			//// We attach the script onto node5 and open it
			//auto script = scriptMng_.selectScript();
			//tree_.attachScript(node5, std::move(script));

			//tree_.save("Scene Data");
			tree_.load("Scene Data");

			auto nodes = tree_.getAllNodes();
			auto vis = nodes[0]->entity_.get<ECSComponents::VisibilityComponent>(entMng_);
			auto lay = nodes[0]->entity_.get<ECSComponents::LayerComponent>(entMng_);

			entMng_.viewMap();
			std::cout << "Visibility Z Index: " << vis.z_index << std::endl;
			std::cout << "Visibility Enabled?: " << vis.visible << std::endl;
			std::cout << "Layer: " << lay.layer << std::endl;


			// Call script delegate to get exposed fields
			// after attaching the script
			//int count{};
			//Core::EnoCoreHost::Field* data = (coreHost_.fields())(L"PlayerController", &count);

			//for (size_t i = 0; i < count; i++)
			//{
			//	Core::EnoCoreHost::Field& field = data[i];

			//	std::wcout << "Name : " << field.name_ << std::endl <<
			//		"Type: " << field.type_ << std::endl <<
			//		"Field: " << field.value_ << std::endl;
			//}
			//
			// Modify value and run a function in C# to verify value change
			// PlayerController script attached
			//node5->script_->displayVars();
			//node5->script_->modValue(L"playerName", L"Anthony");
			//node5->script_->displayVars();
			

			////////////////////////////////////////////////////////////////
			//Core::EnoCoreHost::Field* data1 = (coreHost_.fields())(L"PlayerController", &count);

			//for (size_t i = 0; i < count; i++)
			//{
			//	Core::EnoCoreHost::Field& field = data1[i];

			//	std::wcout << "Name : " << field.name_ << std::endl <<
			//		"Type: " << field.type_ << std::endl <<
			//		"Field: " << field.value_ << std::endl;
			//}
			// Opens the script
			//if (node5->script_)
			//{
			//	scriptMng_.openScript(node5->script_->path());
			//}
		}

		// Create and append a new scene
		void createScene()
		{
			std::unique_ptr<EnoScene> scene = std::make_unique<EnoScene>(bus_);
			scenes_.push_back(std::move(scene));
		}

		void selectScene()
		{
			
		}

		// Plays the selected scene
		void playScene()
		{
			if (!playingScene_)
			{
				playingScene_ = true;

				// Create an additional window
				bus_.publish(CreateGameWindow{});
				bus_.publish(GameInitialized{});
			}
		}

		// Stops playing a running scene
		void stopScene()
		{
			if (playingScene_)
			{
				playingScene_ = false;

				// Destroy the game window
				bus_.publish(DestroyGameWindow{});
				bus_.publish(GameStopped{});
			}
		}

		// Pause a running scene
		void pauseScene()
		{
			if (playingScene_)
			{
				std::cout << "Pausing scene!" << std::endl;
			}
		}

	private:
		// Scenes
		std::vector<std::unique_ptr<EnoScene>> scenes_;
		EnoScene* selectedScene_;

		// Core Host
		ECS::EntityManager& entMng_;

		// Core Host
		Core::EnoCoreHost& coreHost_;

		// Scripting
		Script::EnoScriptManager scriptMng_{ coreHost_ };

		// Events
		Event::EnoEventBus& bus_;

		// Scene tree
		EnoSceneTree tree_{ entMng_, coreHost_ };
		bool playingScene_;
	};
}