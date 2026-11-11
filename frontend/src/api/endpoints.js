const endpoints = {
  auth: {
    login: "/users/login",
    logout: "/users/logout",
    me: "/users/me",
    profile: "/users/me",
    photo: "/users/me/photo",
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

  alerts: {
    list: "/alerts",
    detail: (id) => `/alerts/${id}`,
    acknowledge: (id) => `/alerts/${id}/acknowledge`,
    resolve: (id) => `/alerts/${id}/resolve`,
    delete: (id) => `/alerts/${id}`,
  },

  dashboard: {
    stats: "/dashboard/stats",
    weeklyPassages: "/dashboard/weekly-passages",
  },

  logo: {
    current: "/logo",
    create: "/logo",
    update: "/logo",
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
    updateStatus: (id) => `/locations/${id}/status`,
    delete: (id) => `/locations/${id}`,
  },

  devices: {
    list: "/devices",
    create: "/devices",
    detail: (id) => `/devices/${id}`,
    update: (id) => `/devices/update/${id}`,
    updateStatus: (id) => `/devices/update/status/${id}`,
    delete: (id) => `/devices/${id}`,
  },
};

export default endpoints;
