﻿//---------------------------------------------------------------------
// <copyright file="odata_enum_query_executor.h" company="Microsoft">
//      Copyright (C) Microsoft Corporation. All rights reserved. See License.txt in the project root for license information.
// </copyright>
//---------------------------------------------------------------------

#pragma once

#include "odata/client/odata_client.h"
#include "odata/common/utility.h"
#include "odata/core/odata_core.h"
#include "odata/codegen/odata_service_context.h"
#include "cpprest/uri.h"
#include "cpprest/asyncrt_utils.h"
#include "cpprest/json.h"
#include "cpprest/http_client.h"

namespace odata { namespace codegen {

template<typename ElementType, typename Resolver>
class odata_enum_query_executor
{
public:
	odata_enum_query_executor(std::shared_ptr<::odata::codegen::odata_service_context> client_context) : m_client_context(client_context)
	{
	}

	typedef typename std::pair<std::vector<ElementType>, ::odata::client::http_result> return_type;

	::pplx::task<return_type> execute_query(const ::odata::string_t& query_expression)
	{
		if (!m_client_context || !m_client_context->get_client())
		{
			return ::pplx::task_from_result(std::make_pair(return_type::first_type(), ::odata::client::http_result(::web::http::http_headers(), ::web::http::status_code(-1))));
		}

		return m_client_context->get_client()->get_data_from_server(query_expression).then(
			[this] (const std::pair<std::vector<std::shared_ptr<odata::core::odata_value>>, ::odata::client::http_result>& values) -> return_type
			{
				return_type::first_type vec;

				for(auto iter = values.first.cbegin(); iter != values.first.cend(); ++iter)
				{
					auto enum_value = std::dynamic_pointer_cast<::odata::core::odata_enum_value>(*iter);
					if (enum_value)
					{
						vec.emplace_back(Resolver::get_enum_type_from_string(enum_value->to_string(), ElementType()));
					}
				}

				return std::make_pair(std::move(vec), values.second);
			});
	}

	::pplx::task<return_type> execute_operation_query(const ::odata::string_t& query_expression, const std::vector<std::shared_ptr<::odata::core::odata_parameter>>& parameters, bool is_function)
	{
		if (!m_client_context || !m_client_context->get_client())
		{
			return ::pplx::task_from_result(std::make_pair(return_type::first_type(), ::odata::client::http_result(::web::http::http_headers(), ::web::http::status_code(-1))));
		}

		auto &client_context = m_client_context;
		std::vector<std::shared_ptr<::odata::core::odata_value>> ret_values;
		auto status_code = m_client_context->get_client()->send_data_to_server(query_expression, parameters, ret_values, is_function ? HTTP_GET : HTTP_POST).get().second;

		return_type::first_type vec;
		for(auto iter = ret_values.cbegin(); iter != ret_values.cend(); ++iter)
		{
			auto enum_value = std::dynamic_pointer_cast<::odata::core::odata_enum_value>(*iter);
			if (enum_value)
			{
				vec.emplace_back(Resolver::get_enum_type_from_string(enum_value->to_string(), ElementType()));
			}
		}

		return ::pplx::task_from_result(std::make_pair(std::move(vec), std::move(status_code)));
	}

protected:
	std::shared_ptr<::odata::codegen::odata_service_context> m_client_context;
};

}}
