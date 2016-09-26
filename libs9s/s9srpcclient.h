/*
 * Severalnines Tools
 * Copyright (C) 2016  Severalnines AB
 *
 * This file is part of s9s-tools.
 *
 * s9s-tools is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Foobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar. If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include "S9sString"
#include "S9sRpcReply"

class S9sRpcClientPrivate;

class S9sRpcClient
{
    public:
        S9sRpcClient();

        S9sRpcClient(
                const S9sString &hostName,
                const int        port,
                const S9sString &token);

        S9sRpcClient(const S9sRpcClient &orig);


        virtual ~S9sRpcClient();


        S9sRpcClient &operator=(const S9sRpcClient &rhs);
    
        const S9sRpcReply &reply() const;
        S9sString errorString() const;
        
        /*
         * The executers that send an RPC request and receive an RPC reply from
         * the server.
         */
        bool getClusters();

        bool setHost(
                const int             clusterId,
                const S9sVariantList &hosts,
                const S9sVariantMap  &properties);

        bool getJobInstances(const int clusterId);

        bool getJobInstance(const int clusterId, const int jobId);
        
        bool getJobLog(
                const int clusterId, 
                const int jobId,
                const int limit   = 0,
                const int offset  = 0);

        bool rollingRestart(const int clusterId);

        bool createGaleraCluster(
                const S9sVariantList &hosts,
                const S9sString      &osUserName,
                const S9sString      &vendor,
                const S9sString      &mySqlVersion,
                bool                  uninstall);

        bool createMySqlReplication(
                const S9sVariantList &hosts,
                const S9sString      &osUserName,
                const S9sString      &vendor,
                const S9sString      &mySqlVersion,
                bool                  uninstall);

        bool createNdbCluster(
                const S9sVariantList &mySqlHosts,
                const S9sVariantList &mgmdHosts,
                const S9sVariantList &ndbdHosts,
                const S9sString      &osUserName, 
                const S9sString      &vendor,
                const S9sString      &mySqlVersion,
                bool                  uninstall);

        bool addNode(
                const int             clusterId,
                const S9sVariantList &hosts);

        bool removeNode(
                const int             clusterId,
                const S9sVariantList &hostNames);

        bool stopCluster(
                const int             clusterId);

        bool dropCluster(
                const int             clusterId);

    protected:
        virtual bool 
            executeRequest(
                const S9sString &uri,
                const S9sString &payload);

    private:
        S9sRpcClientPrivate *m_priv;
};

