#pragma once

#include <boost/property_tree/json_parser.hpp>
#include <memory>

class json
{
public:
	json(){}
	virtual ~json(){clean();}

	bool read(const std::string& s)
	{
		clean();

		std::shared_ptr<boost::property_tree::ptree> spTree{new boost::property_tree::ptree};
		std::stringstream stream(s);
		try { boost::property_tree::json_parser::read_json( stream, *spTree ); } catch( ... ) { return false; }

		m_spTree = spTree;
		return true;
	}
	std::string getdata( const std::string& sField ) const 
	{
		return m_spTree ? m_spTree->get( sField, "" ) : "";
	}
protected:
	std::shared_ptr<boost::property_tree::ptree> m_spTree;

	void clean( void ) { m_spTree = nullptr; }
};
