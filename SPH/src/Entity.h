#pragma once
#ifndef ENTITY_H
#define ENTITY_H

#include <entt/entt.hpp>
#include <memory>
#include "Tag.h"

class Simulation;

class Entity {
	private:
		entt::entity entity_handle = entt::null;
		Simulation* sim = nullptr;

	public:
		Entity() = default;
		Entity(entt::entity s_entity_handle, Simulation* s_scene): entity_handle(s_entity_handle), sim(s_scene){}
		Entity(const Entity& other) = default;

		inline entt::entity get_handle() { 
			return entity_handle; 
		}

		template<typename T, typename... Args>
		T& add_component(Args&&... args){
			return sim->registry.emplace<T>(entity_handle, std::forward<Args>(args)...);
		}

		template<typename T>
		void add_tag_component(){
			sim->registry.emplace_or_replace<T>(entity_handle);
		}

		template<typename T>
		T& get_component(){
			return sim->registry.get<T>(entity_handle);
		}

		template<typename T>
		void remove_component(){
			return sim->registry.remove<T>(entity_handle);
		}

		template<typename T>
		bool has_component(){
			return sim->registry.all_of<T>(entity_handle);
		}

		operator bool() const { return entity_handle != entt::null; }
		operator entt::entity() const { return entity_handle; }

		const std::string& get_name() { return get_component<Tag>(); }

		bool operator==(const Entity& other) const{
			return entity_handle == other.entity_handle && sim == other.sim;
		}

		bool operator!=(const Entity& other) const{
			return !(*this == other);
		}
};

#endif
