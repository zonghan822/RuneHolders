template <typename Left, typename Right, typename LH, typename LE, typename RH, typename RE>
void EnoBiMap<Left, Right, LH, LE, RH, RE>::insert(const Left& l, const Right& r)
{
	left[l] = r;
	right[r] = l;
}

template <typename Left, typename Right, typename LH, typename LE, typename RH, typename RE>
std::optional<Left> EnoBiMap<Left, Right, LH, LE, RH, RE>::findr(const Right& r) const
{
	return right.find(r);
}

template <typename Left, typename Right, typename LH, typename LE, typename RH, typename RE>
std::optional<Right> EnoBiMap<Left, Right, LH, LE, RH, RE>::findl(const Left& l) const
{
	return left.find(l);
}

template <typename Left, typename Right, typename LH, typename LE, typename RH, typename RE>
size_t EnoBiMap<Left, Right, LH, LE, RH, RE>::size() const 
{ 
	return left.size();
}
