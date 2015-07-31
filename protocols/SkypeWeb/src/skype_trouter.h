/*
Copyright (c) 2015 Miranda NG project (http://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _SKYPE_TROUTER_H_
#define _SKYPE_TROUTER_H_

struct TRInfo
{
	std::string socketIo,
		connId,
		st,
		se,
		instance,
		ccid,
		sessId,
		sig,
		url;
	time_t lastRegistrationTime;
};

#endif //_SKYPE_TROUTER_H_