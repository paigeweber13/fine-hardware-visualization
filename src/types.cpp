//
// Created by riley on 10/20/20.
//

#include "types.hpp"

// ===== aggregationType and resultType things =====
std::string fhv::types::aggregationTypeToString(
    const fhv::types::aggregation_t &aggregation_type)
{
  if (aggregation_type == fhv::types::aggregation_t::sum)
    return "sum";
  else if (aggregation_type ==
           fhv::types::aggregation_t::arithmetic_mean)
    return "arithmetic_mean";
  else if (aggregation_type ==
           fhv::types::aggregation_t::geometric_mean)
    return "geometric_mean";
  else if (aggregation_type ==
           fhv::types::aggregation_t::saturation)
    return "saturation";
  else
    return "unknown_aggregation_type";
}

std::string fhv::types::resultTypeToString(
    const fhv::types::result_t &result_type)
{
  if (result_type == fhv::types::result_t::event)
    return "event";
  else if (result_type == fhv::types::result_t::metric)
    return "metric";
  else
    return "unknown_result_type";
}

// ===== PerThreadResult function definitions =====
bool
fhv::types::PerThreadResult::operator<(
    const PerThreadResult& other) const
{
  // start by comparing region names
  if (this->region_name != other.region_name)
    return this->region_name < other.region_name;
  else
  {
    // if region names match, order by thread num
    if (this->thread_num != other.thread_num)
      return this->thread_num < other.thread_num;
    else
    {
      // if thread nums match, order by group name
      if(this->group_name != other.group_name)
        return this->group_name < other.group_name;
      else
      {
        // if group names match, order by result type
        if(this->result_type != other.result_type)
          return this->result_type < other.result_type;
        else {
          // if result types match, order by result name
          return this->result_name < other.result_name;
        }
      }
    }
  }
}

std::string fhv::types::PerThreadResult::toString(
    std::string delim) const
{
  std::string result_t_string = resultTypeToString(this->result_type);

  std::stringstream ss;
  ss << std::left << "region " << this->region_name << delim
     << "thread " << std::setw(3) << this->thread_num << delim
     // << "group " << std::setw(15) << this->group_name << delim
     // << std::setw(6) << result_t_string << delim
     << std::setw(40) << this->result_name << delim
     << std::setprecision(4) << std::fixed << std::right << std::setw(20)
     << this->result_value
     << std::endl;
  return ss.str();
}

bool
fhv::types::PerThreadResult::matchesForAggregation(
    const PerThreadResult& other) const
{
  if(this->region_name == other.region_name
     && this->group_name == other.group_name
     && this->result_type == other.result_type
     && this->result_name == other.result_name)
    return true;
  else
    return false;
}

// ===== AggregateResult function definitions =====
bool
fhv::types::AggregateResult::operator<(
    const AggregateResult& other) const
{
  if (this->region_name != other.region_name)
    return this->region_name < other.region_name;
  else
  {
    if (this->aggregation_type != other.aggregation_type)
      return this->aggregation_type < other.aggregation_type;
    else
    {
      if(this->group_name != other.group_name)
        return this->group_name < other.group_name;
      else {
        if(this->result_type != other.result_type)
          return this->result_type < other.result_type;
        else
          return this->result_name < other.result_name;
      }
    }
  }
}

std::string fhv::types::AggregateResult::toString(
    std::string delim) const
{
  std::string result_t_string = resultTypeToString(this->result_type);
  std::string aggregation_t_string = aggregationTypeToString(
      this->aggregation_type);

  std::stringstream ss;
  ss << std::left << "region " << this->region_name << delim
     // << "group " << std::setw(15) << this->group_name << delim
     // << std::setw(6) << result_t_string << delim
     << "aggregation_type " << std::setw(15) << aggregation_t_string << delim
     << std::setw(40) << this->result_name << delim
     << std::setprecision(4) << std::fixed << std::right << std::setw(20)
     << this->result_value
     << std::endl;
  return ss.str();
}

