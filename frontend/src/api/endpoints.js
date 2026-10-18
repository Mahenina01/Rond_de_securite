const endpoints = {
  auth: {
    login: "/users/login",
    logout: "/users/logout",
    me: "/users/me",
    profile: "/users/me",
    password: "/users/me/password",
  },

  agents: {
    list: "/agents",
    create: "/agents",
    detail: (id) => `/agents/${id}`,
    update: (id) => `/agents/${id}`,
    updateStatus: (id) => `/agents/${id}/status`,
    delete: (id) => `/agents/${id}`,
  },

  logs: {
    list: "/logs",
    detail: (id) => `/logs/${id}`,
    delete: (id) => `/logs/${id}`,
  },

  notifications: {
    list: "/notifications",
    detail: (id) => `/notifications/${id}`,
    archive: (id) => `/notifications/${id}/archive`,
  },

  locations: {
    list: "/locations",
    search: "/locations",
    detail: (id) => `/locations/${id}`,
    agentId: (agentId) => `/locations/by-agent/${agentId}`,
    create: "/locations",
    update: (id) => `/locations/${id}`,
    delete: (id) => `/locations/${id}`,
  },

  devices: {
    list: "/devices",
    create: "/devices",
    detail: (id) => `/devices/${id}`,
    update: (id) => `/devices/${id}`,
    delete: (id) => `/devices/${id}`,
  },
};

export default endpoints;
