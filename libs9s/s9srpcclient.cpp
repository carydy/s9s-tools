/* 
 * Copyright (C) 2016 severalnines.com
 */
#include "s9srpcclient.h"
#include "s9srpcclient_p.h"

#include "S9sOptions"

#include <string.h>
#include <stdio.h>

//#define DEBUG
//#define WARNING
#include "s9sdebug.h"

#define READ_SIZE 512

S9sRpcClient::S9sRpcClient() :
    m_priv(new S9sRpcClientPrivate)
{
}

S9sRpcClient::S9sRpcClient(
        const S9sString &hostName,
        const int        port,
        const S9sString &token) :
    m_priv(new S9sRpcClientPrivate)
{
    m_priv->m_hostName = hostName;
    m_priv->m_port     = port;
    m_priv->m_token    = token;
}


/**
 * Copy constructor. Nothing to see here.
 */
S9sRpcClient::S9sRpcClient (
		const S9sRpcClient &orig)
{
	m_priv = orig.m_priv;

	if (m_priv) 
		m_priv->ref ();
}

/**
 * 
 */
S9sRpcClient::~S9sRpcClient()
{
	if (m_priv && m_priv->unRef() == 0)
    {
        delete m_priv;
        m_priv = 0;
	}
}

/**
 * Assignment operator to utilize the implicit sharing.
 */
S9sRpcClient &
S9sRpcClient::operator= (
		const S9sRpcClient &rhs)
{
	if (this == &rhs)
		return *this;

	if (m_priv && m_priv->unRef() == 0)
    {
        delete m_priv;
        m_priv = 0;
	}

	m_priv = rhs.m_priv;
	if (m_priv) 
    {
		m_priv->ref ();
	}

	return *this;
}

/**
 * \returns the reply that received from the controller.
 *
 * The reply the controller sends is a JSON string which is parsed by the
 * S9sRpcClient and presented here as an S9sVariantMap (S9sRpcReply that
 * inherits S9sVariantMap to be more precise).
 */
const S9sRpcReply &
S9sRpcClient::reply() const
{
    return m_priv->m_reply;
}

/**
 * \returns the human readable error string stored in the object.
 */
S9sString 
S9sRpcClient::errorString() const
{
    return m_priv->m_errorString;
}

/**
 * The method that sends the "getAllClusterInfo" RPC request and reads the
 * reply.
 */
bool
S9sRpcClient::getClusters()
{
    S9sOptions    *options = S9sOptions::instance();
    S9sString      uri = "/0/clusters/";
    S9sVariantMap  request;
    bool           retval;

    request["operation"]  = "getAllClusterInfo";
    request["with_hosts"] = true;
    request["user"]           = options->userName();
    //job["user_id"]        = options->userId();
    
    if (!m_priv->m_token.empty())
        request["token"] = m_priv->m_token;

    retval = executeRequest(uri, request.toString());

    return retval;
}

bool
S9sRpcClient::setHost(
        const int             clusterId,
        const S9sVariantList &hostNames,
        const S9sVariantMap  &properties)
{
    S9sString      uri;
    S9sVariantMap  request;

    uri.sprintf("/%d/stat", clusterId);

    if (hostNames.size() != 1u)
    {
        PRINT_ERROR("setHost is currently implemented only for one node.");
        return false;
    }

    request["operation"]  = "setHost";
    request["hostname"]   = hostNames[0].toString();
    // FIXME: No way to handle ports.
    request["port"]       = 3306;
    request["properties"] = properties;
    
    if (!m_priv->m_token.empty())
        request["token"] = m_priv->m_token;

        
    return executeRequest(uri, request.toString());
}


/**
 * Sends a "getJobInstances" request, receives the reply. We use this RPC call
 * to get the job list (e.g. s9s job --list).
 */
bool
S9sRpcClient::getJobInstances(
        const int clusterId)
{
    S9sString      uri;
    S9sVariantMap  request;
    bool           retval;

    uri.sprintf("/%d/job/", clusterId);

    request["operation"] = "getJobInstances";

    if (!m_priv->m_token.empty())
        request["token"] = m_priv->m_token;

    retval = executeRequest(uri, request.toString());

    return retval;
}

/**
 * \param clusterId the ID of the cluster
 * \param jobId the ID of the job
 * \returns true if the operation was successful, a reply is received from the
 *   controller (even if the reply is an error reply).
 *
 * This function sends a "getJobInstance" request to the controller and receives
 * its reply. This request can be used to get the properties of one particular
 * job.
 */
bool
S9sRpcClient::getJobInstance(
        const int clusterId,
        const int jobId)
{
    S9sString      uri;
    S9sVariantMap  request;
    bool           retval;

    uri.sprintf("/%d/job/", clusterId);

    request["operation"] = "getJobInstance";
    request["job_id"]    = jobId;

    if (!m_priv->m_token.empty())
        request["token"] = m_priv->m_token;

    retval = executeRequest(uri, request.toString());

    return retval;
}

/**
 * \param clusterId the ID of the cluster that owns the job
 * \param jobId the ID of the job
 * \param limit the maximum number of log entries we are ready to process
 * \param offset the number of log entries to skip
 * \returns true if the operation was successful, a reply is received from the
 *   controller (even if the reply is an error reply).
 *
 * This function will get the log entries in ascending order. This is because
 * the terminal normally used like that.
 */
bool
S9sRpcClient::getJobLog(
        const int clusterId,
        const int jobId,
        const int limit,
        const int offset)
{
    S9sString      uri;
    S9sVariantMap  request;
    bool           retval;

    uri.sprintf("/%d/job/", clusterId);

    // Building the request.
    request["operation"]  = "getJobLog";
    request["job_id"]     = jobId;
    request["ascending"]  = true;
    if (limit != 0)
        request["limit"]  = limit;

    if (offset != 0)
        request["offset"] = offset;

    if (!m_priv->m_token.empty())
        request["token"]  = m_priv->m_token;

    retval = executeRequest(uri, request.toString());

    return retval;

}

/**
 * \param clusterId the ID of the cluster that will be restarted
 * \returns true if the operation was successful, a reply is received from the
 *   controller (even if the reply is an error reply).
 *
 * Creates a job for "rolling restart" and receives the controller's answer for
 * the request. 
 */
bool
S9sRpcClient::rollingRestart(
        const int clusterId)
{
    S9sOptions    *options = S9sOptions::instance();
    S9sVariantMap  request;
    S9sVariantMap  job, jobSpec;
    S9sString      uri;
    bool           retval;

    uri.sprintf("/%d/job/", clusterId);

    jobSpec["command"]   = "rolling_restart";

    job["class_name"]    = "CmonJobInstance";
    job["title"]         = "Rolling Restart";
    job["job_spec"]      = jobSpec;
    job["user_name"]     = options->userName();
    job["user_id"]       = options->userId();

    request["operation"] = "createJobInstance";
    request["job"]       = job;
    
    if (!m_priv->m_token.empty())
        request["token"] = m_priv->m_token;
    
    retval = executeRequest(uri, request.toString());

    return retval;
}

/**
 * \returns true if the operation was successful, a reply is received from the
 *   controller (even if the reply is an error reply).
 *
 */
bool
S9sRpcClient::createGaleraCluster(
        const S9sVariantList &hostNames,
        const S9sString      &osUserName,
        const S9sString      &vendor,
        const S9sString      &mySqlVersion,
        bool                  uninstall)
{
    S9sOptions    *options = S9sOptions::instance();
    S9sVariantMap  request;
    S9sVariantMap  job, jobData, jobSpec;
    S9sString      uri;
    bool           retval;
    
    uri = "/0/job/";

    // The job_data describing the cluster.
    jobData["cluster_type"]    = "galera";
    jobData["mysql_hostnames"] = hostNames;
    jobData["vendor"]          = vendor;
    jobData["mysql_version"]   = mySqlVersion;
    jobData["enable_mysql_uninstall"] = uninstall;
    jobData["ssh_user"]        = osUserName;
    //jobData["repl_user"]        = options->dbAdminUserName();
    jobData["mysql_password"]  = options->dbAdminPassword();
    
    if (!options->clusterName().empty())
        jobData["cluster_name"] = options->clusterName();
    
    if (!options->osKeyFile().empty())
        jobData["ssh_key"]     = options->osKeyFile();

    // The jobspec describing the command.
    jobSpec["command"]  = "create_cluster";
    jobSpec["job_data"] = jobData;

    // The job instance describing how the job will be executed.
    job["class_name"]    = "CmonJobInstance";
    job["title"]         = "Create Galera Cluster";
    job["job_spec"]      = jobSpec;
    job["user_name"]     = options->userName();
    job["user_id"]       = options->userId();

    // The request describing we want to register a job instance.
    request["operation"] = "createJobInstance";
    request["job"]       = job;
    
    if (!m_priv->m_token.empty())
        request["token"] = m_priv->m_token;

    retval = executeRequest(uri, request.toString());
    
    return retval;
}

/**
 * \returns true if the operation was successful, a reply is received from the
 *   controller (even if the reply is an error reply).
 */
bool
S9sRpcClient::createMySqlReplication(
        const S9sVariantList &hostNames,
        const S9sString      &osUserName,
        const S9sString      &vendor,
        const S9sString      &mySqlVersion,
        bool                  uninstall)
{
    S9sOptions    *options = S9sOptions::instance();
    S9sVariantMap  request;
    S9sVariantMap  job, jobData, jobSpec;
    S9sString      uri = "/0/job/";
    bool           retval;
    
    // The job_data describing the cluster.
    jobData["cluster_type"]     = "replication";
    jobData["mysql_hostnames"]  = hostNames;
    jobData["master_address"]   = hostNames[0].toString();
    jobData["vendor"]           = vendor;
    jobData["mysql_version"]    = mySqlVersion;
    jobData["enable_mysql_uninstall"] = uninstall;
    jobData["type"]             = "mysql";
    jobData["ssh_user"]         = osUserName;
    jobData["repl_user"]        = options->dbAdminUserName();
    jobData["repl_password"]    = options->dbAdminPassword();
   
    if (!options->clusterName().empty())
        jobData["cluster_name"] = options->clusterName();

    if (!options->osKeyFile().empty())
        jobData["ssh_key"]      = options->osKeyFile();

    // The jobspec describing the command.
    jobSpec["command"]  = "create_cluster";
    jobSpec["job_data"] = jobData;

    // The job instance describing how the job will be executed.
    job["class_name"]    = "CmonJobInstance";
    job["title"]         = "Create MySQL Replication Cluster";
    job["job_spec"]      = jobSpec;
    job["user_name"]     = options->userName();
    job["user_id"]       = options->userId();
    //job["api_id"]        = -1;

    // The request describing we want to register a job instance.
    request["operation"] = "createJobInstance";
    request["job"]       = job;
    
    if (!m_priv->m_token.empty())
        request["token"] = m_priv->m_token;

    retval = executeRequest(uri, request.toString());

    return retval;
}

/**
 * Creates a job that will add a new node to the cluster.
 */
bool
S9sRpcClient::addNode(
        const S9sVariantList &hostNames)
{
    S9sOptions    *options   = S9sOptions::instance();
    int            clusterId = options->clusterId();
    S9sVariantMap  request;
    S9sVariantMap  job, jobData, jobSpec;
    S9sString      uri;
    bool           retval;

    if (hostNames.size() != 1u)
    {
        PRINT_ERROR("addnode is currently implemented only for one node.");
        return false;
    }
    
    uri.sprintf("/%d/job/", clusterId);

    // The job_data describing the cluster.
    jobData["hostname"]         = hostNames[0].toString();
    jobData["install_software"] = true;
    jobData["disable_firewall"] = true;
    jobData["disable_selinux"]  = true;
   
    // The jobspec describing the command.
    jobSpec["command"]  = "addnode";
    jobSpec["job_data"] = jobData;

    // The job instance describing how the job will be executed.
    job["class_name"]    = "CmonJobInstance";
    job["title"]         = "Add Node to Cluster";
    job["job_spec"]      = jobSpec;
    job["user_name"]     = options->userName();
    job["user_id"]       = options->userId();
    //job["api_id"]        = -1;

    // The request describing we want to register a job instance.
    request["operation"] = "createJobInstance";
    request["job"]       = job;
    
    if (!m_priv->m_token.empty())
        request["token"] = m_priv->m_token;

    retval = executeRequest(uri, request.toString());

    return retval;
}

/**
 * This function will create a "removeNode" job on the controller.
 */
bool
S9sRpcClient::removeNode(
        const S9sVariantList &hostNames)
{
    S9sOptions    *options   = S9sOptions::instance();
    int            clusterId = options->clusterId();
    S9sString      hostName, title;
    S9sVariantMap  request;
    S9sVariantMap  job, jobData, jobSpec;
    S9sString      uri;
    bool           retval;

    if (hostNames.size() != 1u)
    {
        PRINT_ERROR("removenode is currently implemented only for one node.");
        return false;
    }
    
    uri.sprintf("/%d/job/", clusterId);
    hostName = hostNames[0].toString();
    title.sprintf("Remove '%s' from the Cluster", STR(hostName));

    // The job_data describing the cluster.
    jobData["host"]             = hostName;
    //jobData["port"]             =
   
    // The jobspec describing the command.
    jobSpec["command"]  = "removenode";
    jobSpec["job_data"] = jobData;

    // The job instance describing how the job will be executed.
    job["class_name"]    = "CmonJobInstance";
    job["title"]         = title;
    job["job_spec"]      = jobSpec;
    job["user_name"]     = options->userName();
    job["user_id"]       = options->userId();
    //job["api_id"]        = -1;

    // The request describing we want to register a job instance.
    request["operation"] = "createJobInstance";
    request["job"]       = job;
    
    if (!m_priv->m_token.empty())
        request["token"] = m_priv->m_token;

    retval = executeRequest(uri, request.toString());

    return retval;
}

/**
 * \param uri the file path part of the URL where we send the request
 * \param payload the JSON request string
 * \returns true if everything is ok, false on error.
 */
bool
S9sRpcClient::executeRequest(
        const S9sString &uri,
        const S9sString &payload)
{
    S9sOptions  *options = S9sOptions::instance();    
    S9sString    header;
    int          socketFd = m_priv->connectSocket();
    ssize_t      readLength;
   
    m_priv->m_jsonReply.clear();
    m_priv->m_reply.clear();

    if (socketFd < 0)
        return false;


    header.sprintf(
        "POST %s HTTP/1.0\r\n"
        "Host: %s:%d\r\n"
        "User-Agent: cmonjsclient/1.0\r\n"
        "Connection: close\r\n"
        "Accept: application/json\r\n"
        "Transfer-Encoding: identity\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %u\r\n"
        "\r\n",
        STR(uri), 
        STR(m_priv->m_hostName), 
        m_priv->m_port, 
        payload.length());

    /*
     * Sending the HTTP request header.
     */
    if (m_priv->writeSocket(socketFd, STR(header), header.length()) < 0)
    {
        S9S_DEBUG("Error writing socket %d: %m", socketFd);

        m_priv->m_errorString.sprintf("Error writing socket %d: %m", socketFd);
        m_priv->closeSocket(socketFd);

        return false;
    }

    /*
     * Sending the JSON payload.
     */
    if (!payload.empty())
    {
        if (m_priv->writeSocket(socketFd, STR(payload), payload.length()) < 0)
        {
            m_priv->m_errorString.sprintf(
                    "Error writing socket %d: %m", 
                    socketFd);
       
            m_priv->closeSocket(socketFd);
            return false;
        } else {
            if (options->isJsonRequested() && options->isVerbose())
            {
                printf("Request: \n%s\n", STR(payload));
            }
        }
    }

    /*
     * Reading the reply from the server.
     */
    m_priv->clearBuffer();
    readLength = 0;
    do
    {
        m_priv->ensureHasBuffer(m_priv->m_dataSize + READ_SIZE);

        readLength = m_priv->readSocket(
                socketFd,
                m_priv->m_buffer + m_priv->m_dataSize, 
                READ_SIZE - 1);

        if (readLength > 0)
            m_priv->m_dataSize += readLength;
    } while (readLength > 0);

    // Closing the buffer with a null terminating byte.
    m_priv->ensureHasBuffer(m_priv->m_dataSize + 1);
    m_priv->m_buffer[m_priv->m_dataSize] = '\0';
    m_priv->m_dataSize += 1;
            


    // Closing the socket.
    m_priv->closeSocket(socketFd);
    
    if (m_priv->m_dataSize > 1)
    {
        char *tmp = strstr(m_priv->m_buffer, "\r\n\r\n");

        if (tmp)
        {
            m_priv->m_jsonReply = (tmp + 4);

            if (options->isJsonRequested() && options->isVerbose())
                printf("Reply: \n%s\n", STR(m_priv->m_jsonReply));
        }
    } else {
        m_priv->m_errorString.sprintf(
                "Error reading socket %d: %m", socketFd);
        return false;
    }

    if (!m_priv->m_reply.parse(STR(m_priv->m_jsonReply)))
    {
        PRINT_ERROR("Error parsing JSON reply.");
        m_priv->m_errorString.sprintf("Error parsing JSON reply.");
        return false;
    }

    //printf("-> \n%s\n", STR(m_priv->m_reply.toString()));
    return true;
}
