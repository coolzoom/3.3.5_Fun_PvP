/*
 * Copyright (C) 2008-2017 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BOUTIQUE_MGR
#define BOUTIQUE_MGR

#include "Player.h"
#include <map>

namespace Maelstrom {

	class StoreManager {

	public:

		// Singleton ------------------------------------------------------------------------------------------
		static StoreManager* instance()
		{
			static StoreManager instance;
			return &instance;
		}

		// Structures publiques -------------------------------------------------------------------------------
		struct StoreItem {

			// Rep ---------------------
			uint32 m_id;
			uint32 m_catId;
			uint32 m_itemEntry;
			uint32 m_price;
			uint32 m_quantity;
			uint32 m_requiredAvgItemLevel;
			//--------------------------

			// Constructor -------------
			StoreItem(uint32 id, uint32 catId, uint32 itemEntry, uint32 price, uint32 quantity, uint32 requiredAvgItemLevel) {
				m_id = id;
				m_catId = catId;
				m_itemEntry = itemEntry;
				m_price = price;
				m_quantity = quantity;
				m_requiredAvgItemLevel = requiredAvgItemLevel;
			}
			//--------------------------
		};

		struct StoreCategory {

			// Rep ---------------------
			uint32             m_catId;
			std::string        m_name;      //Nom de la catйgorie dans les diffйrentes langues prises en charge
			//--------------------------

			// Constructor -------------
			StoreCategory(uint32 catId, const std::string name) {
				m_catId = catId;
				m_name = name;
			}
		};
		//-----------------------------------------------------------------------------------------------------

		// Enumйrations publiques -----------------------------------------------------------------------------
		enum PurchaseResult {
			PURCHASE_RESULT_ERROR,
			PURCHASE_RESULT_NOT_ENOUGH_COINS,
			PURCHASE_RESULT_NOT_ENOUGH_FREE_SLOTS,
			PURCHASE_RESULT_SUCCESS
		};

		//-----------------------------------------------------------------------------------------------------

		// Constructeur ---------------------------------------------------------------------------------------
		StoreManager();
		//-----------------------------------------------------------------------------------------------------

		// Mйthodes publiques ---------------------------------------------------------------------------------
		void LoadStore();
		uint32 GetAccountCoins(uint32 accountId) const;
		void SetAccountCoins(uint32 accountId, uint32 newCoinsValue);
		PurchaseResult PurchaseItem(Player* player, uint32 itemEntry);

		std::map<uint32, StoreCategory> GetCategories()									  const;
		std::map<uint32, StoreItem>     GetItems()										  const;
		std::map<uint32, StoreItem>     GetItems(uint32 catId, uint32 playerAvgItemLevel) const;

	private:

		// Mйthodes privйes -----------------------------------------------------------------------------------
		void LoadCategories();
		void LoadItems();
		//-----------------------------------------------------------------------------------------------------


		// Reprйsentation -------------------------------------------------------------------------------------
		std::map<uint32 /*boutique_item_id*/, StoreItem>     m_items;
		std::map<uint32 /*cat_id*/          , StoreCategory> m_categories;

	};

#define sStoreMgr StoreManager::instance()

}

#endif