#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

using namespace geode::prelude;

namespace {
struct DecorationAnchor {
    CCPoint position;
    int objectID;
};

// Small deterministic generator: identical selection + seed produces identical decor.
uint32_t mix(uint32_t value) {
    value ^= value >> 16;
    value *= 0x7feb352dU;
    value ^= value >> 15;
    value *= 0x846ca68bU;
    return value ^ (value >> 16);
}

float unit(uint32_t value) {
    return static_cast<float>(mix(value) & 0x00ffffffU) /
           static_cast<float>(0x01000000U);
}
}

class $modify(AutoDecoratorEditorUI, EditorUI) {
    struct Fields {
        CCMenu* m_autoDecoratorMenu = nullptr;
    };

    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) {
            return false;
        }

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto menu = CCMenu::create();
        menu->setID("auto-decorator-menu"_spr);
        menu->setPosition({winSize.width - 72.f, winSize.height - 74.f});

        auto sprite = ButtonSprite::create(
            "Decorate", 44, true, "bigFont.fnt", "GJ_button_04.png", 25.f, 0.55f
        );
        auto button = CCMenuItemSpriteExtra::create(
            sprite, this, menu_selector(AutoDecoratorEditorUI::onAutoDecorate)
        );
        button->setID("auto-decorate-button"_spr);
        menu->addChild(button);
        this->addChild(menu, 100);
        m_fields->m_autoDecoratorMenu = menu;

        return true;
    }

    void onAutoDecorate(CCObject*) {
        std::vector<DecorationAnchor> anchors;

        if (m_selectedObject) {
            anchors.push_back({m_selectedObject->getPosition(), m_selectedObject->m_objectID});
        }

        if (m_selectedObjects) {
            for (auto* object : CCArrayExt<GameObject*>(m_selectedObjects)) {
                if (!object || object == m_selectedObject) {
                    continue;
                }
                anchors.push_back({object->getPosition(), object->m_objectID});
            }
        }

        if (anchors.empty()) {
            Notification::create(
                "First select objects to decorate", NotificationIcon::Warning, 2.f
            )->show();
            return;
        }

        auto mod = Mod::get();
        auto density = std::clamp(mod->getSettingValue<int64_t>("density"), int64_t(5), int64_t(100));
        auto amount = std::clamp(mod->getSettingValue<int64_t>("amount"), int64_t(1), int64_t(5));
        auto spread = std::max(0.0, mod->getSettingValue<double>("spread"));
        auto minScale = std::max(0.1, mod->getSettingValue<double>("min-scale"));
        auto maxScale = std::max(minScale, mod->getSettingValue<double>("max-scale"));
        auto glowID = static_cast<int>(
            std::clamp(mod->getSettingValue<int64_t>("decoration-object-id"), int64_t(1), int64_t(9999))
        );
        auto whiteChannel = static_cast<int>(
            std::clamp(mod->getSettingValue<int64_t>("white-channel"), int64_t(1), int64_t(999))
        );
        auto blackChannel = static_cast<int>(
            std::clamp(mod->getSettingValue<int64_t>("black-channel"), int64_t(1), int64_t(999))
        );
        auto seed = static_cast<uint32_t>(mod->getSettingValue<int64_t>("seed"));
        auto safetyLimit = static_cast<size_t>(
            std::clamp(mod->getSettingValue<int64_t>("max-created"), int64_t(10), int64_t(3000))
        );

        size_t created = 0;
        for (size_t i = 0; i < anchors.size() && created < safetyLimit; ++i) {
            auto base = mix(seed ^ static_cast<uint32_t>(i * 0x9e3779b9U) ^
                            static_cast<uint32_t>(anchors[i].objectID));

            if (unit(base) * 100.f >= static_cast<float>(density)) {
                continue;
            }

            // A dark oversized core gives every cluster a clean monochrome silhouette.
            if (created < safetyLimit) {
                auto* core = this->createObject(glowID, anchors[i].position);
                if (core) {
                    core->setScale(static_cast<float>(maxScale * 1.45));
                    core->setRotation(unit(base ^ 0x27d4eb2dU) * 45.f);
                    core->setColor({0, 0, 0});
                    core->setChildColor({0, 0, 0});
                    core->setOpacity(210);
                    ++created;
                }
            }

            // White rays are placed in opposite pairs, producing intentional symmetry.
            for (int detail = 0; detail < amount && created < safetyLimit; ++detail) {
                auto key = mix(base ^ static_cast<uint32_t>((detail + 1) * 0x85ebca6bU));
                auto phase = unit(base ^ 0x9e3779b9U) * 0.78539816339f;
                auto angle = phase + static_cast<float>(detail) * 1.0471975512f;
                auto radius = static_cast<float>(spread) *
                    (0.72f + unit(key ^ 0xa511e9b3U) * 0.28f);
                auto scale = static_cast<float>(
                    minScale + (maxScale - minScale) * unit(key ^ 0x63d83595U)
                );

                for (int side : {-1, 1}) {
                    if (created >= safetyLimit) {
                        break;
                    }
                    auto position = anchors[i].position + CCPoint{
                        std::cos(angle) * radius * static_cast<float>(side),
                        std::sin(angle) * radius * static_cast<float>(side)
                    };
                    auto* ray = this->createObject(glowID, position);
                    if (!ray) {
                        continue;
                    }
                    ray->setScale(scale);
                    ray->setRotation(angle * 57.2957795f + (side < 0 ? 180.f : 0.f));
                    ray->setColor({255, 255, 255});
                    ray->setChildColor({255, 255, 255});
                    ray->setOpacity(static_cast<unsigned char>(
    150 + static_cast<int>(unit(key ^ 0xc2b2ae35U) * 85.f)
));
                    ++created;
                }
            }
        }

        if (created == 0) {
            Notification::create(
                "Nothing was created; raise Density", NotificationIcon::Warning, 2.f
            )->show();
            return;
        }

        Notification::create(
            fmt::format("Added {} decoration objects", created),
            NotificationIcon::Success,
            2.f
        )->show();
    }
};
